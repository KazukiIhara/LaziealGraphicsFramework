# Lazieal Graphics Framework

DirectX 12 と C++20 で開発中の、Windows向けリアルタイム描画フレームワークです。

Mesh Shader、Metallic-Roughness PBR、Image-Based Lighting（IBL）、Cascaded Shadow Maps（CSM）などの現代的な描画技術を採用しつつ、利用側では `Draw()` と `SetScene()` を中心に扱える、OpenSiv3Dのような簡潔なAPIを目指しています。

> [!IMPORTANT]
> 現在は学習・研究用の開発途中版です。APIの互換性、配布用ライブラリ、安定したエラーハンドリングはまだ保証していません。

## 現在の主な機能

### 3Dレンダリング

- Mesh Shaderによるプリミティブ描画とインスタンシング
- Albedo / Normal / Material のG-Bufferを使ったDeferred Rendering
- Cook-Torrance BRDF（GGX / Smith / Schlick Fresnel）によるMetallic-Roughness PBR
- ベースカラー、法線、Metallic-Roughness、Ambient Occlusionテクスチャ
- HDRシーンカラー、EV露出補正、ACES Filmic Tone Mapping
- 正距円筒図法HDR環境マップを使ったSkybox
- Diffuse Irradiance、GGX Specular Prefilter、BRDF LUTによるSplit-Sum IBL
- 平行光源
- 4分割Cascaded Shadow Maps
  - 各カスケード 2048 x 2048 / D32
  - Practical Split（Uniform + Logarithmic）
  - カスケード境界のブレンド
  - Texel Snapping
  - 3 x 3 PCF
- Assimpによる静的モデル読み込み
- DirectXMeshによるメッシュレット生成
- Amplification Shaderによるメッシュレット単位の視錐台カリング
- モデルの同一リソース単位でのインスタンスバッチ
- メッシュレット色・境界球、カメラ視錐台、CSM分割のデバッグ描画

### プリミティブ

- 3D: `Triangle3D`、`Box3D`、`Sphere3D`、`Capsule3D`、`Cone3D`、`Cylinder3D`、`Plane3D`、`Ground3D`、`Line3D`、`Grid3D`
- 2D: `Triangle2D`、`Rect2D`、`Circle2D`、`RoundRect2D`、`Line2D`

### アセットと基盤

- WIC対応画像（PNG、JPEG、BMPなど）、DDS、TGA、HDRの同期読み込み
- sRGB / Linearの明示、Mip生成、ロード設定を含むパス単位キャッシュ
- 単一2DテクスチャおよびCubemapのSRV生成
- JSONからのGraphics / Compute Pipeline State生成
- ウィンドウリサイズ時のSwap Chain、Depth、G-Buffer再生成
- キーボード、マウスボタン、Raw Input、カーソル、ホイール入力
- ImGuiのDirectX 12 / Win32バックエンド統合
- ベクトル、行列、Quaternion、座標変換用の数学API

## 動作要件

- Windows 10以降
- Visual Studio 2026（Desktop development with C++）
- Windows SDKとDirectX Shader Compiler（DXC）
- C++20対応MSVCツールセット
- 次の機能をすべて満たすDirectX 12 GPUとドライバー
  - Mesh Shader Tier 1以上
  - Shader Model 6.6以上
  - Resource Binding Tier 3

GPUの世代名ではなく、起動時に上記機能を問い合わせて判定します。要件を満たさない場合は初期化に失敗します。

## ビルドと実行

依存ライブラリは `Project/Externals` に同梱され、Premakeから個別プロジェクトとして生成されます。ワークスペースは描画本体、実行基盤、サンプルを次の3ターゲットに分離しています。

- `LaziealGraphicsFramework`: 描画・数学・GPUリソース管理を提供するStatic Library
- `LaziealRuntime`: Window、Input、Time、フレーム進行を提供するStatic Library
- `Sandbox`: `Main()` を実装するサンプル用WindowedApp（スタートアッププロジェクト）

1. `Project/Premake.bat` を実行します。
2. 生成された `Project/LaziealGraphicsFramework.slnx` を開きます。以前生成された `.sln` が残っている場合は、旧プロジェクト構成なので使用しないでください。
3. `x64` の `Debug`、`Develop`、または `Release` を選択して `Sandbox` をビルドします。
4. `Project` を作業ディレクトリとして `Sandbox` を実行します。

生成物は `generated/outputs/<Configuration>/x64` に出力されます。描画ライブラリはアセット位置を固定で保持せず、Sandboxが返す `RuntimeConfig::assetRoot` をRuntimeが絶対パスへ正規化し、`GraphicsInitDesc` 経由で渡します。モデルやテクスチャなど利用側が直接指定するパスは、引き続き作業ディレクトリからの相対パスを利用できます。

## 最小コード

Runtime Static LibraryがWin32のエントリーポイントを提供します。利用側は同じソースファイルに `ConfigureRuntime()` と `Main()` を定義します。

```cpp
#include <LGF/LGF.h>

using namespace LGF;

RuntimeConfig ConfigureRuntime() {
    RuntimeConfig config{};
    config.assetRoot = "Assets";
    config.window.title = L"My Game";
    config.window.width = 1280;
    config.window.height = 720;
    return config;
}

void Main() {
    Camera3D camera({ 0.0f, 1.5f, -5.0f });
    DirectionalLight light(
        { 1.0f, -1.0f, 0.5f },
        Color::White,
        1.5f);

    const Box3D box;
    const Material3D material{
        .color = Color::Orange,
        .metallic = 0.1f,
        .roughness = 0.45f,
    };

    while (System::Update()) {
        camera.SetScene();
        light.SetScene();
        box.Draw(Math::Identity4x4(), material);
    }
}
```

`System::Update()` がウィンドウイベントとフレーム進行を管理します。3Dオブジェクトはフレーム中の `Draw()` でキューに追加され、フレーム終了時にまとめて描画されます。

## テクスチャ付きマテリアル

```cpp
Texture ormTexture(
    "Assets/Models/FlightHelmet/FlightHelmet_Materials_LeatherPartsMat_OcclusionRoughMetal.png",
    TextureLoadDesc::Data());

Material3D material{
    .color = Color::White,
    .baseColorTexture = Texture("Assets/Textures/Checker.bmp"),
    .normalTexture = Texture(
        "Assets/Models/FlightHelmet/FlightHelmet_Materials_LeatherPartsMat_Normal.png",
        TextureLoadDesc::NormalMap()),
    .metallicRoughnessTexture = ormTexture,
    .occlusionTexture = ormTexture,
    .normalScale = 1.0f,
    .metallic = 1.0f,
    .roughness = 1.0f,
    .occlusionStrength = 1.0f,
    .uvScale = { 4.0f, 4.0f },
};

Box3D().Draw(Math::Identity4x4(), material);
```

- 色画像は既定でsRGBとして読み込みます。
- 法線マップは `TextureLoadDesc::NormalMap()`、ORMやAOなどのデータ画像は `TextureLoadDesc::Data()` でLinearとして読み込みます。
- `baseColorTexture` を省略すると単色描画になります。指定時はテクスチャと `color` を乗算します。
- Metallic-RoughnessテクスチャはglTFと同じくGをRoughness、BをMetallicとして使用します。AOはRチャンネルです。
- `unlit = true` では直接光・IBL・影を適用しません。

テクスチャ読み込みは現在同期処理です。失敗時は `std::exception` 派生の例外を送出し、GPUリソースはフレームワーク終了までキャッシュされます。

## 静的モデル

```cpp
Model model("Assets/Models/FlightHelmet/FlightHelmet.gltf");

ModelDrawOptions options{
    .enableNormalMap = true,
    .normalScale = 1.0f,
    .metallicScale = 1.0f,
    .roughnessScale = 1.0f,
    .occlusionStrength = 1.0f,
};

while (System::Update()) {
    camera.SetScene();
    light.SetScene();
    model.Draw(Math::Identity4x4(), options);
}
```

モデルは初回ロード時にキャッシュされ、頂点を左手座標系へ変換してメッシュレットを生成します。ベースカラー、法線、Metallic-Roughness、AOと各係数を読み込みます。GLB内の圧縮画像と、glTFから参照される外部画像の両方に対応しています。

同じモデルを複数描画する場合は `DrawInstanced()` を利用できます。

```cpp
std::array<ModelInstance3D, 3> instances{};
for (std::size_t i = 0; i < instances.size(); ++i) {
    instances[i].worldMatrix = Math::MakeTranslateMatrix(
        { static_cast<float>(i) * 2.0f, 0.0f, 0.0f });
}
model.DrawInstanced(instances, options);
```

## Skybox、IBL、露出

SkyboxにはLinear HDRの正距円筒図法テクスチャを指定します。初回設定時にDiffuse Irradiance Cubemap、GGX Prefilter Cubemap、BRDF LUTをGPUで生成し、以降はキャッシュして利用します。

```cpp
Skybox skybox(
    "Assets/Textures/Environment/StoryStudio02/story_studio_02_2k.dds");
skybox.intensity = 1.0f;
skybox.rotation = 0.0f;

while (System::Update()) {
    skybox.SetScene();
    Graphics3D::SetExposure(0.0f); // EV。+1で2倍、-1で半分
}
```

`.hdr` をDDSへ変換する補助ツールは [`Project/Tools/Texconv/README.md`](Project/Tools/Texconv/README.md) を参照してください。

## カリング、影、デバッグ表示

通常は描画カメラがカリングとCSMの基準にもなります。別カメラを使う場合のみ、フレームごとに `SetCullingScene()` または `SetShadowScene()` を呼びます。

```cpp
Camera3D renderCamera({ 0.0f, 3.0f, -10.0f });
Camera3D debugCamera({ 0.0f, 12.0f, -20.0f });

while (System::Update()) {
    debugCamera.SetScene();
    renderCamera.SetCullingScene();
    renderCamera.SetShadowScene();
    renderCamera.DrawDebug({
        .showFrustum = true,
        .showShadowCascades = true,
    });

    model.Draw(Math::Identity4x4(), {
        .debugDraw = {
            .showMeshletColors = true,
            .showMeshletBoundingSpheres = true,
        },
    });
}
```

現在のCSMは最大100 mを対象とする固定4分割です。プリミティブと静的モデルは影を落としますが、`Line3D` とメッシュレット境界球などのデバッグ描画は対象外です。

## 入力

```cpp
if (Key::Space.Trigger()) {
    // 押した瞬間
}
if (Key::W.Press()) {
    // 押している間
}
if (Mouse::Right.Release()) {
    // 離した瞬間
}

const Vector2 rawMouseDelta = Mouse::Delta();
const Vector2 cursorPosition = Cursor::Pos();
const float wheel = Mouse::WheelDelta();
```

## 描画パス

現在の1フレームは、おおむね次の順序で処理されます。

1. 3Dプリミティブとモデルを4枚のCSMへ描画
2. 3DジオメトリをG-Bufferへ描画
3. 平行光源、影、IBLを使ってHDR `SceneColor` を合成
4. 露出補正とACES Tone Mappingを適用してSwap Chainへ出力
5. 2DプリミティブとImGuiをLDR出力へ重ねる

Pipeline Stateは `Project/Assets/JsonData/Pipeline` 以下のJSONを起動時に再帰的に読み込み、対応するHLSLをDXCでコンパイルして生成します。

## プロジェクト構成

```text
Project/
├─ Include/LGF/                 描画・数学の公開API
├─ Src/Graphics, Math/          描画Static Libraryの実装
├─ Src/Core, Input, LGF/        Runtimeの実装
├─ Runtime/
│  ├─ Include/                  Runtime側の公開API
│  └─ Src/                      EntryPointなどRuntime固有の実装
├─ Sandbox/
│  └─ main.cpp                  公開APIだけを使うサンプルアプリ
├─ Assets/
│  ├─ JsonData/Pipeline/        Pipeline State定義
│  ├─ Shaders/                  HLSL
│  ├─ Models/                   サンプルモデル
│  └─ Textures/                 サンプルテクスチャ・環境マップ
├─ Externals/                   同梱サードパーティ
├─ Premake/premake5.lua         Graphics / Runtime / Sandbox生成定義
└─ Tools/                       アセット変換補助
```

主な同梱ライブラリはDirectXTex、DirectXMesh、Dear ImGui、Assimp、nlohmann/jsonです。サンプルモデルごとの出典とライセンスは [`Project/Assets/Models/README.md`](Project/Assets/Models/README.md) に記載しています。

## 現在の制限

- GraphicsとRuntimeはStatic Libraryへ分離済みですが、別リポジトリへの分割と配布パッケージ化は未対応です。
- GPU完了を毎フレーム待つため、CPUとGPUが直列化されています。
- 3Dの透過・Alpha Mask、Emissive、複数ライト、スキニング、アニメーション、Morph Targetは未対応です。
- モデルのGPUバッファはUpload Heapに保持され、ロード・転送も同期処理です。
- 2Dは単色プリミティブ中心で、Sprite、フォント、2Dテクスチャ描画は未対応です。
- プリミティブは形状ごとに1フレーム4096インスタンスの固定上限があります。
- CSMの解像度、距離、分割数、Bias、PCFカーネルは公開設定になっていません。
- Pipeline JSONやシェーダーのHot Reloadは未対応です。

優先度を含む今後の計画は [`Todo.md`](Todo.md) を参照してください。
