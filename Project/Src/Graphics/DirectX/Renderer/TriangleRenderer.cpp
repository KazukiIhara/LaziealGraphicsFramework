#include "TriangleRenderer.h"

#include <array>
#include <cassert>
#include <cstring>

#include "Pipeline/PipelineJsonLoader.h"
#include "ShaderCompiler/ShaderCompiler.h"

using namespace LGF;

namespace {

	struct Vertex {
		float position[3];
		float color[4];
	};

}

bool TriangleRenderer::Initialize(
	ID3D12Device* device,
	ShaderCompiler& shaderCompiler,
	const std::filesystem::path& pipelineJsonPath) {

	GraphicsPipelineDesc pipelineDesc{};
	if (!PipelineJsonLoader::Load(pipelineJsonPath, pipelineDesc)) {
		return false;
	}

	if (!pipeline_.Initialize(device, shaderCompiler, pipelineDesc)) {
		return false;
	}

	return CreateVertexBuffer(device);
}

void TriangleRenderer::Draw(ID3D12GraphicsCommandList* commandList) {
	commandList->SetGraphicsRootSignature(pipeline_.GetRootSignature());
	commandList->SetPipelineState(pipeline_.GetPipelineState());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->DrawInstanced(3, 1, 0, 0);
}

bool TriangleRenderer::CreateVertexBuffer(ID3D12Device* device) {
	const std::array<Vertex, 3> vertices = {
		Vertex{{ 0.0f, 0.5f, 0.0f }, { 1.0f, 0.2f, 0.2f, 1.0f }},
		Vertex{{ 0.5f, -0.5f, 0.0f }, { 0.2f, 1.0f, 0.3f, 1.0f }},
		Vertex{{ -0.5f, -0.5f, 0.0f }, { 0.2f, 0.4f, 1.0f, 1.0f }},
	};

	const UINT bufferSize = static_cast<UINT>(sizeof(Vertex) * vertices.size());

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = bufferSize;
	resourceDesc.Height = 1u;
	resourceDesc.DepthOrArraySize = 1u;
	resourceDesc.MipLevels = 1u;
	resourceDesc.SampleDesc.Count = 1u;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer_));

	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	void* mappedData = nullptr;
	hr = vertexBuffer_->Map(0, nullptr, &mappedData);
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	std::memcpy(mappedData, vertices.data(), bufferSize);
	vertexBuffer_->Unmap(0, nullptr);

	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = bufferSize;
	vertexBufferView_.StrideInBytes = static_cast<UINT>(sizeof(Vertex));

	return true;
}
