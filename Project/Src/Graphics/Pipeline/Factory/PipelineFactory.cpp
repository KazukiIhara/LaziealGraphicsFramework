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

	// パイプラインステートを生成
	initParam.pipelineState = CreatePipelineState(
		jsonParam,
		initParam.rootSignature,
		initParam.shaderBlobs);

	return std::make_unique<Pipeline>(initParam);
}

std::unordered_map<std::string, ComPtr<ID3DBlob>> LGF::PipelineFactory::CompileShaders(const PipelineJsonLoader::ShaderParam& shaderParam) {

	std::unordered_map<std::string, ComPtr<ID3DBlob>> shaderBlobs;

	const std::string kShaderDirectory = "Assets/Shaders/";

	// ASをコンパイル
	if (!shaderParam.amplification.empty()) {
		shaderBlobs["AS"] = directX_.GetShaderCompiler()->Compile(
			ToWString(kShaderDirectory + shaderParam.amplification),
			L"as_6_6");
		assert(shaderBlobs["AS"] != nullptr);
	}

	// MSをコンパイル
	if (!shaderParam.mesh.empty()) {
		shaderBlobs["MS"] = directX_.GetShaderCompiler()->Compile(
			ToWString(kShaderDirectory + shaderParam.mesh),
			L"ms_6_6");
		assert(shaderBlobs["MS"] != nullptr);
	}

	// PSをコンパイル
	if (!shaderParam.pixel.empty()) {
		shaderBlobs["PS"] = directX_.GetShaderCompiler()->Compile(
			ToWString(kShaderDirectory + shaderParam.pixel),
			L"ps_6_6");
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

ComPtr<ID3D12PipelineState> LGF::PipelineFactory::CreatePipelineState(
	const PipelineJsonLoader::Param& jsonParam,
	const ComPtr<ID3D12RootSignature>& rootSignature,
	const std::unordered_map<std::string, ComPtr<ID3DBlob>>& shaderBlobs) {

	assert(shaderBlobs.contains("MS"));
	assert(shaderBlobs.contains("PS"));

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
	stream.ms.value.pShaderBytecode = shaderBlobs.at("MS")->GetBufferPointer();
	stream.ms.value.BytecodeLength = shaderBlobs.at("MS")->GetBufferSize();

	stream.ps.type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS;
	stream.ps.value.pShaderBytecode = shaderBlobs.at("PS")->GetBufferPointer();
	stream.ps.value.BytecodeLength = shaderBlobs.at("PS")->GetBufferSize();

	stream.blend.type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND;
	stream.blend.value.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	stream.blend.value.RenderTarget[0].BlendEnable = jsonParam.blend.enable;

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
	stream.primitiveTopology.value = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

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