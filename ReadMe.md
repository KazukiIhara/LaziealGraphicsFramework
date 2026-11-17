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
- Shadow、Middle、Lightの3色と2つの閾値をランタイム指定できるToon Lighting（シーン共通 / マテリアル個別切り替え対応）
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
- Effekseer 1.80.8によるメインパスのエフェクト描画
- メッシュレット境界、カメラ視錐台、CSM分割のデバッグ描画

### Post Process

- Post EffectとProfileをJSONで定義し、Profileの実行時インスタンスを世代付きハンドルで管理
- `BeforeToneMapping`、必須Tone Mapping、`AfterToneMapping`の順にHDR / LDR RenderTextureをピンポンして実行
- Tone Mapping方式と初期パラメータをProfileから選択し、現在はACES FilmicとEV露出補正に対応
- Shader Reflectionと予約名により、直前のColor、HDR SceneColor、Depth、各G-Bufferを必要なShaderへ自動バインド
- Effectごとに8個の`Vector4`パラメータスロットを持ち、C++からEffectの有効状態とパラメータを上書き可能
- 標準EffectとしてBloom、Color Adjustment、Grayscale、Posterization、Sepia、Color Inversion、
  Lens Distortion、Radial Blur、Chromatic Aberration、Vignette、Sharpen、FXAA、Pixelation、Ordered Ditheringを提供

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
- Visual StudioのC++ CMakeツール（初回のEffekseerライブラリ生成に使用）
- 次の機能を満たすDirectX 12対応GPUとドライバー
  - Mesh Shader Tier 1以上
  - Shader Model 6.6以上
  - Resource Binding Tier 3

## プロジェクト生成とビルド

1. `Project/Premake.bat`を実行します。初回のみEffekseerのDebug / Releaseライブラリも自動生成されます。
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

## 設計資料

- [レンダリング機能のパス分類規約](Docs/RenderingFeatureClassification.md)
  - Lighting、Scene Composition、Post Processの責務と、新しい描画機能を追加するときの判断基準を定義します。
- [Post Processシステム仕様](Docs/PostProcessSystem.md)
  - ProfileとEffectのJson形式、Shader予約名、C++ API、標準Effectを定義します。

## 主な依存ライブラリ

- DirectXTex
- DirectXMesh
- Dear ImGui
- Assimp
- nlohmann/json
- Effekseer 1.80.8

## 現在の制限

- 3Dの透過、Alpha Mask、Emissive、複数の平行光源は未対応です。
- ポイントライトの影と、Tiled / Clustered Lightingによるライトカリングは未対応です。
- Skinning、Animation、Morph Targetは未対応です。
- モデルのGPUバッファはUpload Heapに保持され、ロードと転送は同期処理です。
- 2Dは単色プリミティブ中心で、Sprite、Font、2Dテクスチャ描画は未対応です。
- プリミティブは形状ごとに1フレーム4096インスタンスの固定上限があります。
- CSMの解像度、距離、分割数、Bias、PCFカーネルは公開設定になっていません。
- Pipeline JSONとShaderのHot Reloadは未対応です。
- Effekseerはメインパス、同期読み込みに限定し、歪み、深度テクスチャ連携、カスタムRenderPass、非同期読み込みは未対応です。
