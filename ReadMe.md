# Lazieal Graphics Framework

DirectX 12とC++20で開発しているWindows向けリアルタイム描画フレームワークです。
高度な描画機能と、`Draw()` / `SetScene()`を中心とした簡潔な利用APIの両立を目指しています。

> [!IMPORTANT]
> 学習・研究目的で開発中のフレームワークです。
> API互換性や配布パッケージはまだ保証していません。

## 主な機能

### 3Dレンダリング

- Mesh Shaderによる3Dプリミティブ描画とインスタンシング
- Albedo / Normal / Material G-Bufferを利用したDeferred Rendering
- Cook-Torrance BRDFによるMetallic-Roughness PBR
- ベースカラー、法線、Metallic-Roughness、Ambient Occlusionテクスチャ
- HDRシーンカラー、EV露出補正、ACES Filmic Tone Mapping
- SkyboxとSplit-Sum IBL
  - Diffuse Irradiance
  - GGX Specular Prefilter
  - BRDF LUT
- 最大64個のポイントライトと距離減衰
- 平行光源と4分割Cascaded Shadow Maps
  - Practical Split、Texel Snapping、境界ブレンド、3 x 3 PCF
- Assimpによる静的モデル読み込み
- DirectXMeshによるメッシュレット生成
- Amplification Shaderによるメッシュレット単位の視錐台カリング
- モデルのインスタンスバッチ
- メッシュレット境界、カメラ視錐台、CSM分割のデバッグ描画

### プリミティブとUI

- 3D: `Triangle3D`、`Box3D`、`Sphere3D`、`Capsule3D`、`Cone3D`、
  `Cylinder3D`、`Plane3D`、`Ground3D`、`Line3D`、`Grid3D`
- 2D: `Triangle2D`、`Rect2D`、`Circle2D`、`RoundRect2D`、`Line2D`
- Dear ImGuiのDirectX 12バックエンド統合

### アセットと描画基盤

- WIC対応画像、DDS、TGA、HDRの同期読み込み
- sRGB / Linear指定、Mip生成、ロード設定を含むテクスチャキャッシュ
- 2DテクスチャとCubemapのSRV生成
- JSON定義からのGraphics / Compute Pipeline生成
- 2フレーム分のFrame Contextとフレーム別Upload Buffer
- ウィンドウリサイズ時のSwap Chain、Depth、G-Buffer再生成
- ベクトル、行列、Quaternion、座標変換用の数学API

## 動作要件

- Windows 10以降
- Visual Studio 2026とC++20対応MSVCツールセット
- Windows SDKとDirectX Shader Compiler（DXC）
- 次の機能を満たすDirectX 12対応GPUとドライバー
  - Mesh Shader Tier 1以上
  - Shader Model 6.6以上
  - Resource Binding Tier 3

## プロジェクト生成とビルド

1. `Project/Premake.bat`を実行します。
2. 生成された`Project/LaziealGraphicsFramework.slnx`を開きます。
3. x64の`Debug`、`Develop`、または`Release`構成をビルドします。

生成物は`generated/outputs/<Configuration>/x64`に出力されます。

## 利用方法

- Window、Input、Time、エントリーポイントには別リポジトリのLaziealRuntimeを使用します。
- 実行可能なサンプルと検証シーンはLaziealSandboxにあります。
- アプリケーション側は`ConfigureRuntime()`と`Main()`を定義します。
- `RuntimeConfig::assetRoot`にフレームワークの`Project/Assets`を指定します。
- フレームごとにCamera、Light、Skyboxを`SetScene()`し、
  プリミティブやModelを`Draw()`します。

## 主な依存ライブラリ

- DirectXTex
- DirectXMesh
- Dear ImGui
- Assimp
- nlohmann/json

## 現在の制限

- 3Dの透過、Alpha Mask、Emissive、複数の平行光源は未対応です。
- ポイントライトの影と、Tiled / Clustered Lightingによるライトカリングは未対応です。
- Skinning、Animation、Morph Targetは未対応です。
- モデルのGPUバッファはUpload Heapに保持され、ロードと転送は同期処理です。
- 2Dは単色プリミティブ中心で、Sprite、Font、2Dテクスチャ描画は未対応です。
- プリミティブは形状ごとに1フレーム4096インスタンスの固定上限があります。
- CSMの解像度、距離、分割数、Bias、PCFカーネルは公開設定になっていません。
- Pipeline JSONとShaderのHot Reloadは未対応です。
