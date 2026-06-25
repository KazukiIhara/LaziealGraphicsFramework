workspace "LaziealGraphicsFramework"
    configurations { "Debug", "Develop","Release" }
    platforms { "x64" }
    startproject "LaziealGraphicsFramework" -- デフォルトのスタートプロジェクトを指定
    location "../"

    -- 出力ディレクトリの設定
    targetdir "../../generated/outputs/%{cfg.buildcfg}/%{cfg.platform}"  -- 実行ファイルの出力先
    objdir "../../generated/obj/%{prj.name}/%{cfg.buildcfg}"             -- 中間ファイルの出力先

project "LaziealGraphicsFramework" -- プロジェクト名
   kind "WindowedApp" -- デスクトップアプリケーションに設定
   language "C++"
   cppdialect "C++20" -- 言語の設定

   -- プロジェクトに含むファイル
   files { 
      "../*.cpp",

      "../Src/**.cpp",
      "../Src/**.h",

      "../Include/**.cpp",
      "../Include/**.h",

      "../Externals/imgui/*.cpp",
      "../Externals/imgui/*.h",

      "../Externals/imgui/backends/imgui_impl_dx12.cpp",
      "../Externals/imgui/backends/imgui_impl_dx12.h",

      "../Externals/imgui/backends/imgui_impl_win32.cpp",
      "../Externals/imgui/backends/imgui_impl_win32.h",
   }
   
   -- 追加のインクルードパス
   includedirs { 
       "../Src",
       "../Include",
       "../Externals",

       "../Externals/imgui",
       "../Externals/imgui/backends",
   }

   dependson { } -- 依存していることを指定

   links { }  -- リンク対象のプロジェクト

   warnings "High" -- 警告レベル4を設定

   buildoptions { "/utf-8" } -- UTF-8でビルドする設定

   multiprocessorcompile "On" -- 複数プロセッサでのコンパイルを有効化

   -- ビルド後イベントのコマンド
   postbuildcommands {
      'copy "$(WindowsSdkDir)bin\\$(TargetPlatformVersion)\\x64\\dxcompiler.dll" "$(TargetDir)dxcompiler.dll"',
      'copy "$(WindowsSdkDir)bin\\$(TargetPlatformVersion)\\x64\\dxil.dll" "$(TargetDir)dxil.dll"'
   }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"
      fatalwarnings { "All" }  -- 警告をエラーとして扱う
      staticruntime "On"  -- 静的ランタイム（/MTd）
      linkoptions { "/IGNORE:4049", "/IGNORE:4099" } -- 指定したリンカーの警告を無視
      libdirs { "../Externals/assimp/lib/Debug" } -- デバッグ用追加のライブラリディレクトリ
      links { "assimp-vc143-mtd" } -- デバッグ用ライブラリ

   filter "configurations:Develop"
      defines { "DEBUG", "DEVELOP" }     
      symbols "On"                       
      optimize "Debug"                   
      runtime "Release"
      staticruntime "On"                  
      linkoptions { "/IGNORE:4049", "/IGNORE:4099" }
      libdirs { "../Externals/assimp/lib/Release" }  -- Release ライブラリを流用
      links { "assimp-vc143-mt" }

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
      staticruntime "On"  -- 静的ランタイム（/MT）
      linkoptions { "/IGNORE:4049", "/IGNORE:4099" } -- 指定したリンカーの警告を無視
      libdirs { "../Externals/assimp/lib/Release" } -- リリース用追加のライブラリディレクトリ
      links { "assimp-vc143-mt" } -- リリース用ライブラリ
 
