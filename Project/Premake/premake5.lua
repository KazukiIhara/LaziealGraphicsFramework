workspace "LaziealGraphicsFramework"
    configurations { "Debug", "Develop", "Release" }
    platforms { "x64" }
    startproject "LaziealGraphicsFramework"
    location "../"

    -- 出力ディレクトリ
    targetdir "../../generated/outputs/%{cfg.buildcfg}/%{cfg.platform}"
    objdir "../../generated/obj/%{prj.name}/%{cfg.buildcfg}"

project "DirectXTex"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   location "../"
   characterset "Unicode"

   -- DirectXTex本体のソース
   files {
      "../Externals/DirectXTex/DirectXTex/*.cpp",
      "../Externals/DirectXTex/DirectXTex/*.h",
      "../Externals/DirectXTex/DirectXTex/*.inl",
   }

   -- DirectXTex内部で参照するインクルードパス
   includedirs {
      "../Externals/DirectXTex/DirectXTex",
      "../Externals/DirectXTex/Common",
      "../Externals/DirectXTex/DirectXTex/Shaders/Compiled",
   }

   defines {
      "_LIB",
      "_UNICODE",
      "UNICODE",
      "_WIN32_WINNT=0x0A00",
      "_CRT_STDIO_ARBITRARY_WIDE_SPECIFIERS",
   }

   pchheader "DirectXTexP.h"
   pchsource "../Externals/DirectXTex/DirectXTex/DirectXTexUtil.cpp"

   warnings "Off"
   buildoptions { "/utf-8", "/Zc:twoPhase-", "/Zc:__cplusplus" }
   floatingpoint "Fast"
   openmp "On"
   staticruntime "On"
   multiprocessorcompile "On"

   -- GPU圧縮用の埋め込みシェーダーを生成
   prebuildcommands {
      'pushd "%{wks.location}Externals\\DirectXTex\\DirectXTex\\Shaders" && call CompileShaders.cmd && popd'
   }

   filter "configurations:Debug"
      defines { "_DEBUG" }
      symbols "On"
      runtime "Debug"

   filter "configurations:Develop"
      defines { "NDEBUG", "PROFILE" }
      symbols "On"
      optimize "Debug"
      runtime "Release"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
      runtime "Release"

   filter {}

project "Texconv"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   location "../"
   characterset "Unicode"
   targetname "texconv"

   files {
      "../Externals/DirectXTex/Texconv/texconv.cpp",
      "../Externals/DirectXTex/Texconv/ExtendedBMP.cpp",
      "../Externals/DirectXTex/Texconv/PortablePixMap.cpp",
      "../Externals/DirectXTex/Texconv/Texconv.rc",
      "../Externals/DirectXTex/Common/CmdLineHelpers.h",
      "../Externals/DirectXTex/Common/directx.ico",
      "../Tools/Texconv/**.bat",
      "../Tools/Texconv/**.md",
   }

   includedirs {
      "../Externals/DirectXTex/Texconv",
      "../Externals/DirectXTex/Common",
      "../Externals/DirectXTex/DirectXTex",
   }

   dependson { "DirectXTex" }
   links {
      "DirectXTex",
      "ole32",
      "windowscodecs",
      "uuid",
      "version",
   }

   defines {
      "WIN32",
      "_CONSOLE",
      "_UNICODE",
      "UNICODE",
      "_WIN32_WINNT=0x0A00",
   }

   warnings "High"
   buildoptions { "/utf-8", "/Zc:twoPhase-", "/Zc:__cplusplus" }
   floatingpoint "Fast"
   openmp "On"
   staticruntime "On"
   multiprocessorcompile "On"

   filter "configurations:Debug"
      defines { "_DEBUG" }
      symbols "On"
      runtime "Debug"

   filter "configurations:Develop"
      defines { "NDEBUG", "PROFILE" }
      symbols "On"
      optimize "Debug"
      runtime "Release"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
      runtime "Release"

   filter {}

project "ImGui"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"

   files {
      "../Externals/imgui/*.cpp",
      "../Externals/imgui/*.h",
      "../Externals/imgui/backends/imgui_impl_dx12.cpp",
      "../Externals/imgui/backends/imgui_impl_dx12.h",
      "../Externals/imgui/backends/imgui_impl_win32.cpp",
      "../Externals/imgui/backends/imgui_impl_win32.h",
   }

   includedirs {
      "../Externals/imgui",
      "../Externals/imgui/backends",
   }

   warnings "High"
   buildoptions { "/utf-8" }
   staticruntime "On"
   multiprocessorcompile "On"

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"
      runtime "Debug"

   filter "configurations:Develop"
      defines { "DEBUG", "DEVELOP" }
      symbols "On"
      optimize "Debug"
      runtime "Release"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
      runtime "Release"

   filter {}

-- Microsoft DirectXMesh may2026 (MIT)
project "DirectXMesh"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   location "../"
   characterset "Unicode"

   files {
      "../Externals/DirectXMesh/DirectXMesh/*.cpp",
      "../Externals/DirectXMesh/DirectXMesh/*.h",
      "../Externals/DirectXMesh/DirectXMesh/*.inl",
   }

   includedirs {
      "../Externals/DirectXMesh/DirectXMesh",
   }

   defines {
      "_LIB",
      "_UNICODE",
      "UNICODE",
      "_WIN32_WINNT=0x0A00",
      "_CRT_STDIO_ARBITRARY_WIDE_SPECIFIERS",
   }

   warnings "Off"
   buildoptions { "/utf-8", "/Zc:__cplusplus" }
   floatingpoint "Fast"
   staticruntime "On"
   multiprocessorcompile "On"

   filter "configurations:Debug"
      defines { "_DEBUG" }
      symbols "On"
      runtime "Debug"

   filter "configurations:Develop"
      defines { "NDEBUG", "PROFILE" }
      symbols "On"
      optimize "Debug"
      runtime "Release"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
      runtime "Release"

   filter {}

project "LaziealGraphicsFramework"
   kind "WindowedApp"
   language "C++"
   cppdialect "C++20"

   -- Windows.h の min/max マクロと標準ライブラリの衝突を防ぐ
   defines { "NOMINMAX" }

   -- プロジェクトに含めるファイル
   files {
      "../*.cpp",

      "../Src/**.cpp",
      "../Src/**.h",

      "../Include/**.cpp",
      "../Include/**.h",

   }

   -- 追加のインクルードパス
   includedirs {
       "../Src",
       "../Include",
       "../Externals",
	   "../Externals/assimp/include",
	   "../Externals/DirectXMesh/DirectXMesh",

       "../Externals/imgui",
       "../Externals/imgui/backends",

       "../Externals/DirectXTex/DirectXTex",
   }

   dependson { "DirectXTex", "DirectXMesh", "ImGui" }

   links { "DirectXTex", "DirectXMesh", "ImGui" }

   warnings "High"

   buildoptions { "/utf-8" }

   multiprocessorcompile "On"

   -- ビルド後イベント
   postbuildcommands {
      'copy "$(WindowsSdkDir)bin\\$(TargetPlatformVersion)\\x64\\dxcompiler.dll" "$(TargetDir)dxcompiler.dll"',
      'copy "$(WindowsSdkDir)bin\\$(TargetPlatformVersion)\\x64\\dxil.dll" "$(TargetDir)dxil.dll"'
   }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"
      fatalwarnings { "All" }
      staticruntime "On"
      linkoptions { "/IGNORE:4049", "/IGNORE:4099" }
      libdirs { "../Externals/assimp/lib/Debug" }
      links { "assimp-vc143-mtd" }

   filter "configurations:Develop"
      defines { "DEBUG", "DEVELOP" }
      symbols "On"
      optimize "Debug"
      runtime "Release"
      staticruntime "On"
      linkoptions { "/IGNORE:4049", "/IGNORE:4099" }
      libdirs { "../Externals/assimp/lib/Release" }
      links { "assimp-vc143-mt" }

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
      staticruntime "On"
      linkoptions { "/IGNORE:4049", "/IGNORE:4099" }
      libdirs { "../Externals/assimp/lib/Release" }
      links { "assimp-vc143-mt" }
