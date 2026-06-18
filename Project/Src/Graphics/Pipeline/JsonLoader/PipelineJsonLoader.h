#pragma once

#include <vector>
#include <string>
#include <optional>

namespace LGF {

	/// <summary>
	/// Jsonのパイプライン情報をロードする
	/// </summary>
	class PipelineJsonLoader {
	public:
		struct ShaderParam {
			std::string amplification;
			std::string mesh;
			std::string vertex;
			std::string pixel;
			std::string compute;
		};

		struct RasterizerParam {
			std::string cullMode = "Back";
			std::string fillMode = "Solid";
		};

		struct BlendParam {
			bool enable = false;
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

		Param Load(const std::string& filePath);

	private:
		std::string LoadTextFile(const std::string& filePath);
	};

}