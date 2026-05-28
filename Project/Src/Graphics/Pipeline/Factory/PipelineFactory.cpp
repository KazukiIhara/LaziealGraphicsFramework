#include "PipelineFactory.h"

#include <cassert>

#include "Graphics/DirectX/Device/Device.h"
#include "Graphics/DirectX/ShaderCompiler/ShaderCompiler.h"
#include "Graphics/Pipeline/Pipeline.h"
#include "Graphics/Pipeline/PipelineDesc.h"
#include "Graphics/Pipeline/JsonLoader/PipelineJsonLoader.h"

using namespace LGF;

PipelineFactory::PipelineFactory(Device& device, ShaderCompiler& shaderCompiler)
	: device_(device), shaderCompiler_(shaderCompiler) {
}

void PipelineFactory::Initialize() {
}

std::unique_ptr<Pipeline> PipelineFactory::CreatePipeline(const std::string& jsonPath) {
	PipelineJsonLoader jsonLoader;
	PipelineDesc desc = jsonLoader.Load(jsonPath);

	switch (desc.type) {
	case PipelineType::Graphics:
		return CreateGraphicsPipeline(desc);

	case PipelineType::Compute:
		return CreateComputePipeline(desc);
	}

	assert(false);
	return nullptr;
}

std::unique_ptr<Pipeline> PipelineFactory::CreateGraphicsPipeline(const PipelineDesc& desc) {
	auto pipeline = std::make_unique<Pipeline>();

	pipeline->SetName(desc.name);
	pipeline->SetType(PipelineType::Graphics);

	ComPtr<ID3DBlob> vertexShader = nullptr;
	ComPtr<ID3DBlob> pixelShader = nullptr;

	for (const ShaderDesc& shader : desc.shaders) {
		ComPtr<ID3DBlob> shaderBlob = shaderCompiler_.Compile(
			shader.path,
			shader.entryPoint.c_str(),
			shader.profile.c_str());

		pipeline->AddShaderBlob(shader.name, shaderBlob);

		if (shader.name == "vs") {
			vertexShader = shaderBlob;
		} else if (shader.name == "ps") {
			pixelShader = shaderBlob;
		}
	}

	assert(vertexShader != nullptr);
	assert(pixelShader != nullptr);

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> signatureBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob,
		&errorBlob);
	assert(SUCCEEDED(hr));

	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = device_.GetDevice()->CreateRootSignature(
		0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.pRootSignature = rootSignature.Get();

	psoDesc.VS = {
		vertexShader->GetBufferPointer(),
		vertexShader->GetBufferSize()
	};

	psoDesc.PS = {
		pixelShader->GetBufferPointer(),
		pixelShader->GetBufferSize()
	};

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleDesc.Count = 1;

	ComPtr<ID3D12PipelineState> pipelineState = nullptr;
	hr = device_.GetDevice()->CreateGraphicsPipelineState(
		&psoDesc,
		IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(hr));

	pipeline->SetRootSignature(rootSignature);
	pipeline->SetPipelineState(pipelineState);

	return pipeline;
}

std::unique_ptr<Pipeline> PipelineFactory::CreateComputePipeline(const PipelineDesc& desc) {
	auto pipeline = std::make_unique<Pipeline>();

	pipeline->SetName(desc.name);
	pipeline->SetType(PipelineType::Compute);

	ComPtr<ID3DBlob> computeShader = nullptr;

	for (const ShaderDesc& shader : desc.shaders) {
		ComPtr<ID3DBlob> shaderBlob = shaderCompiler_.Compile(
			shader.path,
			shader.entryPoint.c_str(),
			shader.profile.c_str());

		pipeline->AddShaderBlob(shader.name, shaderBlob);

		if (shader.name == "cs") {
			computeShader = shaderBlob;
		}
	}

	assert(computeShader != nullptr);

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};

	ComPtr<ID3DBlob> signatureBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob,
		&errorBlob);
	assert(SUCCEEDED(hr));

	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = device_.GetDevice()->CreateRootSignature(
		0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.CS = {
		computeShader->GetBufferPointer(),
		computeShader->GetBufferSize()
	};

	ComPtr<ID3D12PipelineState> pipelineState = nullptr;
	hr = device_.GetDevice()->CreateComputePipelineState(
		&psoDesc,
		IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(hr));

	pipeline->SetRootSignature(rootSignature);
	pipeline->SetPipelineState(pipelineState);

	return pipeline;
}
