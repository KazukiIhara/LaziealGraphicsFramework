#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include <optional>

#include <nlohmann/json.hpp>

namespace LGF {

	/// <summary>
	/// Jsonのパイプライン情報をロードする
	/// </summary>
	class PipelineJsonLoader {
	public:
		struct ShaderParam {
			std::filesystem::path amplification;
			std::filesystem::path mesh;
			std::filesystem::path vertex;
			std::filesystem::path pixel;
			std::filesystem::path compute;
		};

		struct RasterizerParam {
			std::string cullMode = "Back";
			std::string fillMode = "Solid";
		};

		struct BlendParam {
			std::string mode = "None";
		};

		struct DepthParam {
			bool enable = true;
			bool write = true;
			std::string func = "LessEqual";
		};

		struct RenderTargetParam {
			std::vector<std::string> formats{
				"R8G8B8A8_UNORM_SRGB"
			};
		};

		struct DepthStencilParam {
			std::string format = "D24_UNORM_S8_UINT";
		};

		struct Param {
			std::string name;
			std::string pipelineKind = "MeshGraphics";
			ShaderParam shaders;
			RasterizerParam rasterizer;
			BlendParam blend;
			DepthParam depth;
			RenderTargetParam renderTarget;
			DepthStencilParam depthStencil;
			std::string primitiveTopology = "Triangle";
		};

	public:
		PipelineJsonLoader();
		~PipelineJsonLoader();

		Param Load(const std::filesystem::path& filePath);

	private:
		nlohmann::json LoadJson(const std::filesystem::path& filePath);
		std::string LoadTextFile(const std::filesystem::path& filePath);
	};

}
