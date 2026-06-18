#include "PipelineFactory.h"

#include <cassert>
#include <vector>

#include "Pipeline/Pipeline.h"
#include "DirectX/Core/DirectXCore.h"
#include "DirectX/ShaderCompiler/ShaderCompiler.h"

using namespace LGF;

namespace {

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

	// パイプラインステートを生成
	initParam.pipelineState = CreatePipelineState(
		jsonParam,
		initParam.rootSignature,
		initParam.shaderBlobs);

	return std::make_unique<Pipeline>(initParam);
}

std::unordered_map<std::string, ComPtr<ID3DBlob>> LGF::PipelineFactory::CompileShaders(const PipelineJsonLoader::ShaderParam& shaderParam) {
	
	std::unordered_map<std::string, ComPtr<ID3DBlob>> shaderBlobs;

	// ASをコンパイル
	if (!shaderParam.amplification.empty()) {
		shaderBlobs["AS"] = directX_.GetShaderCompiler()->Compile(
			ToWString(shaderParam.amplification),
			L"as_6_5");
		assert(shaderBlobs["AS"] != nullptr);
	}

	// MSをコンパイル
	if (!shaderParam.mesh.empty()) {
		shaderBlobs["MS"] = directX_.GetShaderCompiler()->Compile(
			ToWString(shaderParam.mesh),
			L"ms_6_5");
		assert(shaderBlobs["MS"] != nullptr);
	}

	// PSをコンパイル
	if (!shaderParam.pixel.empty()) {
		shaderBlobs["PS"] = directX_.GetShaderCompiler()->Compile(
			ToWString(shaderParam.pixel),
			L"ps_6_5");
		assert(shaderBlobs["PS"] != nullptr);
	}

	return shaderBlobs;
}

ComPtr<ID3D12RootSignature> PipelineFactory::CreateRootSignature(
	const std::unordered_map<std::string, ComPtr<ID3DBlob>>& shaderBlobs) {

	shaderBlobs;

	HRESULT hr;

	// TriangleData用のSRV
	D3D12_DESCRIPTOR_RANGE triangleRange{};
	triangleRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	triangleRange.NumDescriptors = 1;
	triangleRange.BaseShaderRegister = 0;
	triangleRange.RegisterSpace = 0;
	triangleRange.OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParams[2]{};

	// CameraData
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_MESH;
	rootParams[0].Descriptor.ShaderRegister = 0;

	// TriangleData
	rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_MESH;
	rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParams[1].DescriptorTable.pDescriptorRanges = &triangleRange;

	D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
	rootSigDesc.NumParameters = _countof(rootParams);
	rootSigDesc.pParameters = rootParams;
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	ComPtr<ID3DBlob> sigBlob;
	ComPtr<ID3DBlob> errorBlob;

	// ルートシグネチャをシリアライズ
	hr = D3D12SerializeRootSignature(
		&rootSigDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&sigBlob,
		&errorBlob);
	assert(SUCCEEDED(hr));

	ComPtr<ID3D12RootSignature> rootSignature;

	// ルートシグネチャを生成
	hr = directX_.GetDevice()->CreateRootSignature(
		0,
		sigBlob->GetBufferPointer(),
		sigBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	return rootSignature;
}

ComPtr<ID3D12PipelineState> LGF::PipelineFactory::CreatePipelineState(const PipelineJsonLoader::Param& jsonParam, const ComPtr<ID3D12RootSignature>& rootSignature, const std::unordered_map<std::string, ComPtr<ID3DBlob>>& shaderBlobs) {
	assert(shaderBlobs.contains("MS"));
	assert(shaderBlobs.contains("PS"));

	struct PipelineStateStream {
		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE rootSignatureType;
		ID3D12RootSignature* rootSignature;

		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE msType;
		D3D12_SHADER_BYTECODE ms;

		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE psType;
		D3D12_SHADER_BYTECODE ps;

		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE blendType;
		D3D12_BLEND_DESC blend;

		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE rasterizerType;
		D3D12_RASTERIZER_DESC rasterizer;

		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE depthStencilType;
		D3D12_DEPTH_STENCIL_DESC depthStencil;

		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE primitiveTopologyType;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopology;

		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE rtvFormatsType;
		D3D12_RT_FORMAT_ARRAY rtvFormats;

		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE dsvFormatType;
		DXGI_FORMAT dsvFormat;

		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE sampleDescType;
		DXGI_SAMPLE_DESC sampleDesc;
	};

	PipelineStateStream stream{};

	// ルートシグネチャを設定
	stream.rootSignatureType = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE;
	stream.rootSignature = rootSignature.Get();

	// MSを設定
	stream.msType = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS;
	stream.ms = {
		shaderBlobs.at("MS")->GetBufferPointer(),
		shaderBlobs.at("MS")->GetBufferSize()
	};

	// PSを設定
	stream.psType = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS;
	stream.ps = {
		shaderBlobs.at("PS")->GetBufferPointer(),
		shaderBlobs.at("PS")->GetBufferSize()
	};

	// ブレンド設定
	stream.blendType = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND;
	stream.blend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	stream.blend.RenderTarget[0].BlendEnable = jsonParam.blend.enable;

	// ラスタライザ設定
	stream.rasterizerType = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER;
	stream.rasterizer.FillMode = ToFillMode(jsonParam.rasterizer.fillMode);
	stream.rasterizer.CullMode = ToCullMode(jsonParam.rasterizer.cullMode);
	stream.rasterizer.FrontCounterClockwise = FALSE;
	stream.rasterizer.DepthClipEnable = TRUE;

	// 深度設定
	stream.depthStencilType = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL;
	stream.depthStencil.DepthEnable = jsonParam.depth.enable;
	stream.depthStencil.DepthWriteMask =
		jsonParam.depth.write ?
		D3D12_DEPTH_WRITE_MASK_ALL :
		D3D12_DEPTH_WRITE_MASK_ZERO;
	stream.depthStencil.DepthFunc = ToDepthFunc(jsonParam.depth.func);

	// プリミティブ設定
	stream.primitiveTopologyType =
		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY;
	stream.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// RTV設定
	stream.rtvFormatsType = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS;
	stream.rtvFormats.NumRenderTargets =
		static_cast<UINT>(jsonParam.renderTarget.formats.size());

	for (uint32_t i = 0; i < jsonParam.renderTarget.formats.size(); ++i) {
		stream.rtvFormats.RTFormats[i] =
			ToFormat(jsonParam.renderTarget.formats[i]);
	}

	// DSV設定
	stream.dsvFormatType = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT;
	stream.dsvFormat = ToFormat(jsonParam.depthStencil.format);

	// サンプル設定
	stream.sampleDescType = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC;
	stream.sampleDesc.Count = 1;
	stream.sampleDesc.Quality = 0;

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