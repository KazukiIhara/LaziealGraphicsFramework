#include "GraphicsPipeline.h"

#include <cassert>

#include "Logger/Logger.h"
#include "ShaderCompiler/ShaderCompiler.h"

using namespace LGF;

bool GraphicsPipeline::Initialize(
	ID3D12Device* device,
	ShaderCompiler& shaderCompiler,
	const GraphicsPipelineDesc& desc) {

	if (!CreateRootSignature(device)) {
		return false;
	}

	return CreatePipelineState(device, shaderCompiler, desc);
}

ID3D12PipelineState* GraphicsPipeline::GetPipelineState()const {
	return pipelineState_.Get();
}

ID3D12RootSignature* GraphicsPipeline::GetRootSignature()const {
	return rootSignature_.Get();
}

bool GraphicsPipeline::CreateRootSignature(ID3D12Device* device) {
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> signatureBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob,
		&errorBlob);

	if (FAILED(hr)) {
		if (errorBlob) {
			Logger::Log(static_cast<const char*>(errorBlob->GetBufferPointer()));
		}
		assert(false);
		return false;
	}

	hr = device->CreateRootSignature(
		0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_));

	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	return true;
}

bool GraphicsPipeline::CreatePipelineState(
	ID3D12Device* device,
	ShaderCompiler& shaderCompiler,
	const GraphicsPipelineDesc& desc) {

	ComPtr<ID3DBlob> vertexShader = shaderCompiler.Compile(
		desc.vertexShader.path,
		desc.vertexShader.entryPoint.c_str(),
		desc.vertexShader.profile.c_str());
	ComPtr<ID3DBlob> pixelShader = shaderCompiler.Compile(
		desc.pixelShader.path,
		desc.pixelShader.entryPoint.c_str(),
		desc.pixelShader.profile.c_str());

	semanticNames_.clear();
	semanticNames_.reserve(desc.inputLayout.size());

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
	inputElements.reserve(desc.inputLayout.size());

	for (const InputElementDesc& element : desc.inputLayout) {
		semanticNames_.push_back(element.semanticName);

		D3D12_INPUT_ELEMENT_DESC inputElement{};
		inputElement.SemanticName = semanticNames_.back().c_str();
		inputElement.SemanticIndex = 0u;
		inputElement.Format = element.format;
		inputElement.InputSlot = element.slot;
		inputElement.AlignedByteOffset = element.offset;
		inputElement.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		inputElement.InstanceDataStepRate = 0u;
		inputElements.push_back(inputElement);
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};
	pipelineStateDesc.pRootSignature = rootSignature_.Get();
	pipelineStateDesc.VS = {
		vertexShader->GetBufferPointer(),
		vertexShader->GetBufferSize()
	};
	pipelineStateDesc.PS = {
		pixelShader->GetBufferPointer(),
		pixelShader->GetBufferSize()
	};
	pipelineStateDesc.BlendState.AlphaToCoverageEnable = false;
	pipelineStateDesc.BlendState.IndependentBlendEnable = false;
	pipelineStateDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	pipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	pipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	pipelineStateDesc.RasterizerState.FrontCounterClockwise = false;
	pipelineStateDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	pipelineStateDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	pipelineStateDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	pipelineStateDesc.RasterizerState.DepthClipEnable = true;
	pipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	pipelineStateDesc.DepthStencilState.DepthEnable = true;
	pipelineStateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	pipelineStateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	pipelineStateDesc.DepthStencilState.StencilEnable = false;
	pipelineStateDesc.InputLayout = {
		inputElements.data(),
		static_cast<UINT>(inputElements.size())
	};
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.NumRenderTargets = 1u;
	pipelineStateDesc.RTVFormats[0] = desc.renderTargetFormat;
	pipelineStateDesc.DSVFormat = desc.depthStencilFormat;
	pipelineStateDesc.SampleDesc.Count = 1u;

	HRESULT hr = device->CreateGraphicsPipelineState(
		&pipelineStateDesc,
		IID_PPV_ARGS(&pipelineState_));

	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	return true;
}
