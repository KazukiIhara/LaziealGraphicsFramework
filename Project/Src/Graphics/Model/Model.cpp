#include "LGF/Graphics/Model.h"

#include <cassert>
#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "LGF/Graphics/Graphics3D.h"
#include "ModelManager.h"
#include "ModelResource.h"
#include "ModelSystemInternal.h"

namespace {
	LGF::ModelManager* g_modelManager = nullptr;
}

void LGF::ModelSystem::Internal::SetModelManager(ModelManager* modelManager) {
	assert(modelManager != nullptr);
	g_modelManager = modelManager;
}

void LGF::ModelSystem::Internal::ResetModelManager() {
	g_modelManager = nullptr;
}

LGF::Model::Model(const std::filesystem::path& filePath) :
	filePath_(filePath) {

	if (!g_modelManager) {
		throw std::runtime_error("Model must be loaded after framework initialization.");
	}
	resource_ = g_modelManager->Load(filePath);
}

LGF::Model::~Model() = default;

void LGF::Model::Draw(
	const Matrix4x4& worldMatrix,
	const ModelDrawOptions& options) const {

	if (!resource_) {
		throw std::runtime_error("Cannot draw an empty Model.");
	}
	if (resource_->rig) {
		throw std::runtime_error("A skinned Model must be drawn with a ModelAnimator.");
	}
	Graphics3D::Draw(*this, worldMatrix, options.color, options);
}

void LGF::Model::Draw(
	const Matrix4x4& worldMatrix,
	const ModelAnimator& animator,
	const ModelDrawOptions& options) const {

	if (!resource_ || !animator.resource_) {
		throw std::runtime_error("Cannot draw a skinned Model without a valid ModelAnimator.");
	}
	if (animator.resource_->model.get() != resource_.get()) {
		throw std::invalid_argument("ModelAnimator belongs to a different Model.");
	}
	Graphics3D::Draw(*this, animator, worldMatrix, options.color, options);
}

void LGF::Model::DrawInstanced(
	std::span<const ModelInstance3D> instances,
	const ModelDrawOptions& options) const {

	if (!resource_) {
		throw std::runtime_error("Cannot draw an empty Model.");
	}
	if (resource_->rig) {
		throw std::runtime_error("A skinned Model must be drawn with a ModelAnimator.");
	}
	for (const ModelInstance3D& instance : instances) {
		Graphics3D::Draw(
			*this,
			instance.worldMatrix,
			instance.color * options.color,
			options);
	}
}

void LGF::Model::DrawInstanced(
	std::span<const ModelInstance3D> instances,
	const ModelAnimator& animator,
	const ModelDrawOptions& options) const {

	if (!resource_) {
		throw std::runtime_error("Cannot draw an empty Model.");
	}
	for (const ModelInstance3D& instance : instances) {
		Draw(instance.worldMatrix, animator, ModelDrawOptions{
			.color = instance.color * options.color,
			.enableNormalMap = options.enableNormalMap,
			.normalScale = options.normalScale,
			.metallicScale = options.metallicScale,
			.roughnessScale = options.roughnessScale,
			.occlusionStrength = options.occlusionStrength,
			.unlit = options.unlit,
			.debugDraw = options.debugDraw,
		});
	}
}
const std::filesystem::path& LGF::Model::GetFilePath() const {
	return filePath_;
}

bool LGF::Model::IsValid() const {
	return resource_ != nullptr;
}

namespace {
	template<class T, class Interpolator>
	T SampleKeys(
		const std::vector<LGF::ModelAnimationKeyframe<T>>& keys,
		float time,
		const T& fallback,
		Interpolator interpolate) {

		if (keys.empty()) {
			return fallback;
		}
		if (keys.size() == 1 || time <= keys.front().time) {
			return keys.front().value;
		}
		const auto next = std::upper_bound(
			keys.begin(), keys.end(), time,
			[](float value, const auto& key) { return value < key.time; });
		if (next == keys.end()) {
			return keys.back().value;
		}
		const auto previous = next - 1;
		const float range = next->time - previous->time;
		const float t = range > 0.0f ? (time - previous->time) / range : 0.0f;
		return interpolate(previous->value, next->value, t);
	}

	void UpdatePalette(LGF::ModelAnimatorResource& animator) {
		const auto& rig = *animator.model->rig;
		for (size_t nodeIndex = 0; nodeIndex < rig.nodes.size(); ++nodeIndex) {
			const auto& transform = animator.localTransforms[nodeIndex];
			LGF::Matrix4x4 matrix = LGF::Math::MakeAffineMatrix(
				transform.scale, transform.rotation, transform.translation);
			const int32_t parentIndex = rig.nodes[nodeIndex].parentIndex;
			if (parentIndex >= 0) {
				matrix = matrix * animator.globalMatrices[static_cast<size_t>(parentIndex)];
			}
			animator.globalMatrices[nodeIndex] = matrix;
		}

		animator.palette[0] = {
			.positionMatrix = LGF::Math::Identity4x4(),
			.normalMatrix = LGF::Math::Identity4x4(),
		};
		for (size_t boneIndex = 0; boneIndex < rig.bones.size(); ++boneIndex) {
			const auto& bone = rig.bones[boneIndex];
			const LGF::Matrix4x4 matrix =
				bone.inverseBindMatrix * animator.globalMatrices[bone.nodeIndex] * rig.inverseRootMatrix;
			animator.palette[boneIndex + 1] = {
				.positionMatrix = matrix,
				.normalMatrix = LGF::Math::Transpose(LGF::Math::Inverse(matrix)),
			};
		}
	}
}

LGF::ModelAnimator::ModelAnimator(const Model& model) {
	if (!model.resource_ || !model.resource_->rig) {
		throw std::invalid_argument("ModelAnimator requires a model with a skeleton or animation.");
	}
	resource_ = std::make_shared<ModelAnimatorResource>();
	resource_->model = model.resource_;
	for (const ModelNodeResource& node : model.resource_->rig->nodes) {
		resource_->localTransforms.push_back(node.bindTransform);
	}
	resource_->globalMatrices.resize(model.resource_->rig->nodes.size());
	resource_->palette.resize(model.resource_->rig->bones.size() + 1);
	resource_->meshes.resize(model.resource_->meshes.size());
	UpdatePalette(*resource_);
}

LGF::ModelAnimator::~ModelAnimator() = default;

bool LGF::ModelAnimator::Play(
	std::string_view animationName,
	const ModelAnimationPlayOptions& options) {

	if (!resource_) {
		return false;
	}
	const auto& rig = *resource_->model->rig;
	const auto it = rig.animationIndices.find(std::string(animationName));
	if (it == rig.animationIndices.end()) {
		return false;
	}
	resource_->animationIndex = it->second;
	resource_->time = 0.0f;
	resource_->playbackSpeed = options.playbackSpeed;
	resource_->fadeSourceTransforms = resource_->localTransforms;
	resource_->fadeTime = 0.0f;
	resource_->fadeDuration = std::max(options.fadeDuration, 0.0f);
	resource_->loop = options.loop;
	resource_->playing = true;
	Update(0.0f);
	return true;
}

void LGF::ModelAnimator::Stop() {
	if (resource_) {
		resource_->playing = false;
	}
}

void LGF::ModelAnimator::Update(float deltaTime) {
	if (!resource_ || resource_->animationIndex == SIZE_MAX) {
		return;
	}
	const auto& rig = *resource_->model->rig;
	const ModelAnimationClip& clip = rig.animations.at(resource_->animationIndex);
	if (resource_->playing) {
		resource_->time += deltaTime * resource_->playbackSpeed;
		resource_->fadeTime += std::abs(deltaTime);
		if (clip.duration > 0.0f) {
			if (resource_->loop) {
				resource_->time = std::fmod(resource_->time, clip.duration);
				if (resource_->time < 0.0f) {
					resource_->time += clip.duration;
				}
			} else {
				resource_->time = std::clamp(resource_->time, 0.0f, clip.duration);
				if (resource_->time == 0.0f || resource_->time == clip.duration) {
					resource_->playing = false;
				}
			}
		}
	}

	for (size_t nodeIndex = 0; nodeIndex < rig.nodes.size(); ++nodeIndex) {
		resource_->localTransforms[nodeIndex] = rig.nodes[nodeIndex].bindTransform;
	}
	for (const ModelAnimationTrack& track : clip.tracks) {
		auto& transform = resource_->localTransforms.at(track.nodeIndex);
		transform.scale = SampleKeys(track.scales, resource_->time, transform.scale,
			[](const Vector3& a, const Vector3& b, float t) { return a + (b - a) * t; });
		transform.rotation = SampleKeys(track.rotations, resource_->time, transform.rotation,
			[](const Quaternion& a, const Quaternion& b, float t) { return Math::Slerp(a, b, t); });
		transform.translation = SampleKeys(track.translations, resource_->time, transform.translation,
			[](const Vector3& a, const Vector3& b, float t) { return a + (b - a) * t; });
	}
	if (resource_->fadeDuration > 0.0f &&
		resource_->fadeSourceTransforms.size() == resource_->localTransforms.size()) {
		const float fade = std::clamp(
			resource_->fadeTime / resource_->fadeDuration,
			0.0f,
			1.0f);
		for (size_t nodeIndex = 0; nodeIndex < resource_->localTransforms.size(); ++nodeIndex) {
			const ModelNodeTransform& source = resource_->fadeSourceTransforms[nodeIndex];
			ModelNodeTransform& destination = resource_->localTransforms[nodeIndex];
			destination.scale = source.scale + (destination.scale - source.scale) * fade;
			destination.rotation = Math::Slerp(source.rotation, destination.rotation, fade);
			destination.translation = source.translation +
				(destination.translation - source.translation) * fade;
		}
		if (fade >= 1.0f) {
			resource_->fadeDuration = 0.0f;
			resource_->fadeSourceTransforms.clear();
		}
	}
	UpdatePalette(*resource_);
}

std::vector<std::string> LGF::ModelAnimator::GetAnimationNames() const {
	std::vector<std::string> result;
	if (!resource_) {
		return result;
	}
	for (const ModelAnimationClip& clip : resource_->model->rig->animations) {
		result.push_back(clip.name);
	}
	return result;
}

std::string_view LGF::ModelAnimator::GetCurrentAnimationName() const {
	if (!resource_ || resource_->animationIndex == SIZE_MAX) {
		return {};
	}
	return resource_->model->rig->animations.at(resource_->animationIndex).name;
}

bool LGF::ModelAnimator::IsPlaying() const {
	return resource_ && resource_->playing;
}

bool LGF::ModelAnimator::IsValid() const {
	return resource_ != nullptr;
}
