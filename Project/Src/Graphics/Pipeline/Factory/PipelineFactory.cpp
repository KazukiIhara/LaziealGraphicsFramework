#include "PipelineFactory.h"

#include <cassert>
#include <vector>

#include <dxcapi.h>
#include <d3d12shader.h>
#include <algorithm>

#include "Pipeline/Pipeline.h"
#include "DirectX/Core/DirectXCore.h"
#include "DirectX/ShaderCompiler/ShaderCompiler.h"

using namespace LGF;

namespace {

	enum class PipelineKind {
		Graphics,
		MeshGraphics,
		Compute,
	};

	std::wstring ToWString(const std::string& text) {
		return std::wstring(text.begin(), text.end());
	}

	DXGI_FORMAT ToFormat(const std::string& format) {
		if (format == "R8G8B8A8_UNORM_SRGB") {
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		}

		if (format == "R8G8B8A8_UNORM") {
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		if (format == "R16G16B16A16_FLOAT") {
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		}

		if (format == "D24_UNORM_S8_UINT") {
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		}

		assert(false);
		return DXGI_FORMAT_UNKNOWN;
	}

	D3D12_CULL_MODE ToCullMode(const std::string& cullMode) {
		if (cullMode == "None") {
			return D3D12_CULL_MODE_NONE;
		}

		if (cullMode == "Front") {
			return D3D12_CULL_MODE_FRONT;
		}

		if (cullMode == "Back") {
			return D3D12_CULL_MODE_BACK;
		}

		assert(false);
		return D3D12_CULL_MODE_BACK;
	}

	D3D12_FILL_MODE ToFillMode(const std::string& fillMode) {
		if (fillMode == "Solid") {
			return D3D12_FILL_MODE_SOLID;
		}

		if (fillMode == "Wireframe") {
			return D3D12_FILL_MODE_WIREFRAME;
		}

		assert(false);
		return D3D12_FILL_MODE_SOLID;
	}

	D3D12_COMPARISON_FUNC ToDepthFunc(const std::string& func) {
		if (func == "Less") {
			return D3D12_COMPARISON_FUNC_LESS;
		}

		if (func == "LessEqual") {
			return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		}

		if (func == "Always") {
			return D3D12_COMPARISON_FUNC_ALWAYS;
		}

		assert(false);
		return D3D12_COMPARISON_FUNC_LESS_EQUAL;
	}

	ShaderBlob* FindShaderBlob(
		std::vector<ShaderBlob>& shaderBlobs,
		const std::string& stageName) {

		for (ShaderBlob& shaderBlob : shaderBlobs) {
			if (shaderBlob.stageName == stageName) {
				return &shaderBlob;
			}
		}

		return nullptr;
	}

	const ShaderBlob* FindShaderBlob(
		const std::vector<ShaderBlob>& shaderBlobs,
		const std::string& stageName) {

		for (const ShaderBlob& shaderBlob : shaderBlobs) {
			if (shaderBlob.stageName == stageName) {
				return &shaderBlob;
			}
		}

		return nullptr;
	}

	bool HasShaderBlob(
		const std::vector<ShaderBlob>& shaderBlobs,
		const std::string& stageName) {

		return FindShaderBlob(shaderBlobs, stageName) != nullptr;
	}

	PipelineKind ToPipelineKind(const std::string& kind) {
		if (kind == "Graphics") {
			return PipelineKind::Graphics;
		}

		if (kind == "MeshGraphics") {
			return PipelineKind::MeshGraphics;
		}

		if (kind == "Compute") {
			return PipelineKind::Compute;
		}

		assert(false);
		return PipelineKind::MeshGraphics;
	}

	D3D12_BLEND_DESC ToBlendDesc(const std::string& blendMode) {
		D3D12_BLEND_DESC desc{};
		desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		if (blendMode == "None") {
			return desc;
		}

		if (blendMode == "Normal") {
			desc.RenderTarget[0].BlendEnable = TRUE;
			desc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			desc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			desc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
			desc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
			return desc;
		}

		assert(false);
		return desc;
	}

	D3D12_PRIMITIVE_TOPOLOGY_TYPE ToPrimitiveTopologyType(const std::string& primitiveTopology) {
		if (primitiveTopology == "Triangle") {
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		}

		if (primitiveTopology == "Line") {
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		}

		if (primitiveTopology == "Point") {
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		}

		assert(false);
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	}

#pragma warning(push)
#pragma warning(disable : 4324)

	template<class T>
	struct alignas(void*) PipelineStreamSubobject {
		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type;
		T value;
	};

#pragma warning(pop)
}

PipelineFactory::PipelineFactory(DirectXCore& directX)
	: directX_(directX) {
}

PipelineFactory::~PipelineFactory() = default;

std::unique_ptr<Pipeline> PipelineFactory::CreatePipeline(const std::string& jsonPath) {
	// jsonからパラメータをロード
	PipelineJsonLoader loader;
	const PipelineJsonLoader::Param jsonParam = loader.Load(jsonPath);

	// パイプライン初期化情報を作成
	Pipeline::InitParam initParam{};
	initParam.name = jsonParam.name;

	// シェーダーをコンパイル
	initParam.shaderBlobs = CompileShaders(jsonParam.shaders);

	// ルートシグネチャを生成
	initParam.rootSignature = CreateRootSignature(initParam.shaderBlobs);

	// パイプライン種別を取得
	const PipelineKind kind = ToPipelineKind(jsonParam.pipelineKind);

	switch (kind) {
		case PipelineKind::MeshGraphics:
			// MeshGraphics用PSOを生成
			initParam.pipelineState = CreateMeshGraphicsPipelineState(
				jsonParam,
				initParam.rootSignature,
				initParam.shaderBlobs);
			break;

		default:
			assert(false);
			break;
	}

	return std::make_unique<Pipeline>(std::move(initParam));
}

std::vector<ShaderBlob> LGF::PipelineFactory::CompileShaders(const PipelineJsonLoader::ShaderParam& shaderParam) {

	std::vector<ShaderBlob> shaderBlobs;

	const std::string kShaderDirectory = "Assets/Shaders/";

	// ASをコンパイル
	if (!shaderParam.amplification.empty()) {
		ShaderBlob shaderBlob{};
		shaderBlob.stageName = "AS";
		shaderBlob.blob = directX_.GetShaderCompiler()->Compile(
			ToWString(kShaderDirectory + shaderParam.amplification),
			L"as_6_6");
		assert(shaderBlob.blob != nullptr);

		shaderBlobs.push_back(shaderBlob);
	}

	// MSをコンパイル
	if (!shaderParam.mesh.empty()) {
		ShaderBlob shaderBlob{};
		shaderBlob.stageName = "MS";
		shaderBlob.blob = directX_.GetShaderCompiler()->Compile(
			ToWString(kShaderDirectory + shaderParam.mesh),
			L"ms_6_6");
		assert(shaderBlob.blob != nullptr);

		shaderBlobs.push_back(shaderBlob);
	}

	// PSをコンパイル
	if (!shaderParam.pixel.empty()) {
		ShaderBlob shaderBlob{};
		shaderBlob.stageName = "PS";
		shaderBlob.blob = directX_.GetShaderCompiler()->Compile(
			ToWString(kShaderDirectory + shaderParam.pixel),
			L"ps_6_6");
		assert(shaderBlob.blob != nullptr);

		shaderBlobs.push_back(shaderBlob);
	}

	return shaderBlobs;
}

ComPtr<ID3D12RootSignature> PipelineFactory::CreateRootSignature(const std::vector<ShaderBlob>& shaderBlobs) {
	// リフレクションで見つけたリソース情報を一時保存する構造体
	struct ResourceInfo {
		D3D_SHADER_INPUT_TYPE type;			// CBV / SRV / Texture/ Samplerなど
		UINT bindPoint;						// b0 / t0 の数字部分
		UINT bindCount;						// 配列数
		UINT space;							// registerのスペース
		D3D12_SHADER_VISIBILITY visibility;	// どのシェーダー用か
	};

	// 全シェーダーから見つけたリソースをまとめる配列
	std::vector<ResourceInfo> resources;

	// シェーダーblobを全走査
	for (const auto& [stageName, blob] : shaderBlobs) {
		// 一旦ALLで初期化
		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL;
		// シェーダーステージを判定
		if (stageName == "MS") {
			visibility = D3D12_SHADER_VISIBILITY_MESH;
		} else if (stageName == "PS") {
			visibility = D3D12_SHADER_VISIBILITY_PIXEL;
		} else if (stageName == "AS") {
			visibility = D3D12_SHADER_VISIBILITY_AMPLIFICATION;
		}

		// リフレクション
		DxcBuffer reflectionBuffer{};
		reflectionBuffer.Ptr = blob->GetBufferPointer();
		reflectionBuffer.Size = blob->GetBufferSize();
		reflectionBuffer.Encoding = DXC_CP_ACP;

		// リフレクションの結果を受け取るオブジェクト
		ComPtr<ID3D12ShaderReflection> reflection;

		// シェーダーリフレクションを作成
		HRESULT hr = directX_.GetShaderCompiler()->GetUtils()->CreateReflection(
			&reflectionBuffer,
			IID_PPV_ARGS(&reflection));
		assert(SUCCEEDED(hr));

		D3D12_SHADER_DESC shaderDesc{};

		// シェーダー情報を取得
		hr = reflection->GetDesc(&shaderDesc);
		assert(SUCCEEDED(hr));

		// シェーダーが使っているリソースを全走査
		for (UINT i = 0; i < shaderDesc.BoundResources; ++i) {
			// リソース1個分の詳細を取得
			D3D12_SHADER_INPUT_BIND_DESC bindDesc{};
			hr = reflection->GetResourceBindingDesc(i, &bindDesc);
			assert(SUCCEEDED(hr));

			ResourceInfo info{};
			info.type = bindDesc.Type;
			info.bindPoint = bindDesc.BindPoint;
			info.bindCount = bindDesc.BindCount;
			info.space = bindDesc.Space;
			info.visibility = visibility;

			// リソースをリストに追加
			resources.push_back(info);
		}
	}

	// RootParameterの並び順をそろえる
	std::sort(resources.begin(), resources.end(),
		[](const ResourceInfo& a, const ResourceInfo& b) {
			if (a.space != b.space) {
				return a.space < b.space;
			}

			if (a.type != b.type) {
				return a.type < b.type;
			}

			return a.bindPoint < b.bindPoint;
		});

	// RootSignature生成用のパラメータ
	std::vector<D3D12_ROOT_PARAMETER> rootParams;
	// DescriptorTable用のレンジ
	std::vector<D3D12_DESCRIPTOR_RANGE> descriptorRanges;

	// 再確保を防ぐ
	rootParams.reserve(resources.size());
	descriptorRanges.reserve(resources.size());

	// Reflection結果からRootParameterを生成
	for (const ResourceInfo& resource : resources) {
		D3D12_ROOT_PARAMETER rootParam{};

		// ConstantBuffer
		if (resource.type == D3D_SIT_CBUFFER) {
			// CBVを設定
			rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			rootParam.ShaderVisibility = resource.visibility;
			rootParam.Descriptor.ShaderRegister = resource.bindPoint;
			rootParam.Descriptor.RegisterSpace = resource.space;

			rootParams.push_back(rootParam);
			continue;
		}

		// StructuredBuffer / Texture
		if (resource.type == D3D_SIT_STRUCTURED ||
			resource.type == D3D_SIT_TEXTURE) {

			D3D12_DESCRIPTOR_RANGE range{};

			// SRVレンジを設定
			range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			range.NumDescriptors = resource.bindCount;
			range.BaseShaderRegister = resource.bindPoint;
			range.RegisterSpace = resource.space;
			range.OffsetInDescriptorsFromTableStart =
				D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			descriptorRanges.push_back(range);

			// DescriptorTableを設定
			rootParam.ParameterType =
				D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParam.ShaderVisibility = resource.visibility;
			rootParam.DescriptorTable.NumDescriptorRanges = 1;
			rootParam.DescriptorTable.pDescriptorRanges =
				&descriptorRanges.back();

			rootParams.push_back(rootParam);
			continue;
		}

		assert(false);
	}

	// RootSignature情報を設定
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
	rootSigDesc.NumParameters = static_cast<UINT>(rootParams.size());
	rootSigDesc.pParameters = rootParams.data();
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	// RootSignatureをシリアライズ
	ComPtr<ID3DBlob> sigBlob;
	ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(
		&rootSigDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&sigBlob,
		&errorBlob);
	assert(SUCCEEDED(hr));

	// RootSignatureを生成
	ComPtr<ID3D12RootSignature> rootSignature;
	hr = directX_.GetDevice()->CreateRootSignature(
		0,
		sigBlob->GetBufferPointer(),
		sigBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	return rootSignature;
}

ComPtr<ID3D12PipelineState> LGF::PipelineFactory::CreateMeshGraphicsPipelineState(const PipelineJsonLoader::Param& jsonParam, const ComPtr<ID3D12RootSignature>& rootSignature, const std::vector<ShaderBlob>& shaderBlobs) {

	const ShaderBlob* meshShader = FindShaderBlob(shaderBlobs, "MS");
	const ShaderBlob* pixelShader = FindShaderBlob(shaderBlobs, "PS");

	assert(meshShader != nullptr);
	assert(pixelShader != nullptr);

	struct PipelineStateStream {
		PipelineStreamSubobject<ID3D12RootSignature*> rootSignature;
		PipelineStreamSubobject<D3D12_SHADER_BYTECODE> ms;
		PipelineStreamSubobject<D3D12_SHADER_BYTECODE> ps;
		PipelineStreamSubobject<D3D12_BLEND_DESC> blend;
		PipelineStreamSubobject<D3D12_RASTERIZER_DESC> rasterizer;
		PipelineStreamSubobject<D3D12_DEPTH_STENCIL_DESC> depthStencil;
		PipelineStreamSubobject<D3D12_PRIMITIVE_TOPOLOGY_TYPE> primitiveTopology;
		PipelineStreamSubobject<D3D12_RT_FORMAT_ARRAY> rtvFormats;
		PipelineStreamSubobject<DXGI_FORMAT> dsvFormat;
		PipelineStreamSubobject<DXGI_SAMPLE_DESC> sampleDesc;
	};

	PipelineStateStream stream{};

	stream.rootSignature.type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE;
	stream.rootSignature.value = rootSignature.Get();

	stream.ms.type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS;
	stream.ms.value.pShaderBytecode = meshShader->blob->GetBufferPointer();
	stream.ms.value.BytecodeLength = meshShader->blob->GetBufferSize();

	stream.ps.type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS;
	stream.ps.value.pShaderBytecode = pixelShader->blob->GetBufferPointer();
	stream.ps.value.BytecodeLength = pixelShader->blob->GetBufferSize();

	stream.blend.type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND;
	stream.blend.value = ToBlendDesc(jsonParam.blend.mode);

	stream.rasterizer.type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER;
	stream.rasterizer.value.FillMode = ToFillMode(jsonParam.rasterizer.fillMode);
	stream.rasterizer.value.CullMode = ToCullMode(jsonParam.rasterizer.cullMode);
	stream.rasterizer.value.FrontCounterClockwise = FALSE;
	stream.rasterizer.value.DepthClipEnable = TRUE;

	stream.depthStencil.type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL;
	stream.depthStencil.value.DepthEnable = jsonParam.depth.enable;
	stream.depthStencil.value.DepthWriteMask =
		jsonParam.depth.write
		? D3D12_DEPTH_WRITE_MASK_ALL
		: D3D12_DEPTH_WRITE_MASK_ZERO;
	stream.depthStencil.value.DepthFunc = ToDepthFunc(jsonParam.depth.func);

	stream.primitiveTopology.type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY;
	stream.primitiveTopology.value = ToPrimitiveTopologyType(jsonParam.primitiveTopology);

	stream.rtvFormats.type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS;
	stream.rtvFormats.value.NumRenderTargets =
		static_cast<UINT>(jsonParam.renderTarget.formats.size());

	for (uint32_t i = 0; i < jsonParam.renderTarget.formats.size(); ++i) {
		stream.rtvFormats.value.RTFormats[i] =
			ToFormat(jsonParam.renderTarget.formats[i]);
	}

	stream.dsvFormat.type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT;
	stream.dsvFormat.value = ToFormat(jsonParam.depthStencil.format);

	stream.sampleDesc.type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC;
	stream.sampleDesc.value.Count = 1;
	stream.sampleDesc.value.Quality = 0;
	D3D12_PIPELINE_STATE_STREAM_DESC streamDesc{};
	streamDesc.SizeInBytes = sizeof(stream);
	streamDesc.pPipelineStateSubobjectStream = &stream;

	ComPtr<ID3D12Device2> device2;

	// MeshShader用のDeviceを取得
	HRESULT hr = directX_.GetDevice()->QueryInterface(IID_PPV_ARGS(&device2));
	assert(SUCCEEDED(hr));

	ComPtr<ID3D12PipelineState> pipelineState;

	// パイプラインステートを生成
	hr = device2->CreatePipelineState(
		&streamDesc,
		IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(hr));

	return pipelineState;
}