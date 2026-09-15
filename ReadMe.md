# Lazieal Graphics Framework

### 必要要件
・Win10 以上　DirectX12 Ultimate 対応GPU (RTX2080以降推奨)
### 設計理念
・すべてコードベースでの実装(ImGUiは利用可能にするが、デバッグ用で機能としては実装しない)  
・アセットの管理をうまく行おうとしない(この上に作るゲームエンジン層にすべて任せる)  
・1機能1関数を徹底する
### 機能
・MSを使ってプリミティブ形状をGPUで生成することで、プリミティブをすべてインスタンシング描画し、ドローコールを削減する  
・スタティックメッシュに対してメシュレット単位での視錐台カリングを行い、描画負荷を軽減する  
・エンジン設定を外部ファイル化　ウィンドウ設定やサイズ、解像度、アンチエイリアス、LOD、影の品質、反射などを外部ファイルで設定できるようにする（シージを参考にする）  
・パイプラインの設定をjson化　設定を読み込んでパイプラインを生成する方式にする  
・レンダーテクスチャの設定をjson化　描画時に板ポリに貼ったりなど、テクスチャとして自由に利用できるようにする  
・ディファードレンダリングの実装  
・平行光源のカスケードシャドウマップを実装  
・ソフトシャドウの実装  
・法線マップの実装  
・Metallic-Roughness PBRの実装
・SkyboxとImage-Based Lightingの実装
・スペキュラーマップの実装  


### テクスチャ付き3D描画

`Main()` 内でテクスチャを読み込み、`Material3D` を `Draw()` に渡します。

```cpp
Texture ormTexture(
    "Assets/Models/FlightHelmet/FlightHelmet_Materials_LeatherPartsMat_OcclusionRoughMetal.png",
    TextureLoadDesc::Data());

Material3D material{};
material.baseColorTexture = Texture("Assets/Textures/Checker.bmp");
material.normalTexture = Texture(
    "Assets/Models/FlightHelmet/FlightHelmet_Materials_LeatherPartsMat_Normal.png",
    TextureLoadDesc::NormalMap());
material.metallicRoughnessTexture = ormTexture;
material.occlusionTexture = ormTexture;
material.normalScale = 1.0f;
material.metallic = 1.0f;
material.roughness = 1.0f;
material.occlusionStrength = 1.0f;
material.unlit = false;
material.color = Color::White;
material.uvScale = { 4.0f, 4.0f };

while (System::Update()) {
    Camera3D(Vector3(0.0f, 1.0f, -5.0f)).SetScene();
    Box3D().Draw(Math::Identity4x4(), material);
}
```

- `Triangle3D`・`Box3D`・`Plane3D`・`Ground3D`・`Sphere3D`・`Capsule3D`・`Cone3D`・`Cylinder3D`・`Line3D` に対応しています。従来の色指定 `Draw(matrix, color)` も使用できます。
- `baseColorTexture` 未指定時は単色、指定時はテクスチャと `color` を乗算します。`normalTexture` はタンジェント空間法線マップで、`normalScale` は `0` で無効、`1` で元の強さです。
- Metallic-Roughness PBRでは、`metallicRoughnessTexture` のGをRoughness、BをMetallicとして使用し、`occlusionTexture` のRをAOとして使用します。同じORM画像を両方へ指定できます。`metallic`・`roughness` はテクスチャへ乗算され、`occlusionStrength` はAOの強度です。
- ライティングはCook-Torrance BRDF（GGX・Smith・Schlick Fresnel）による平行光源の直接光です。`unlit = true` ではライト計算を行わずベースカラーをそのまま出力します。現在は不透明描画が対象で、透過とIBLによる環境反射は未実装です。
- `Line3D::Draw(matrix, color)` はデバッグ表示へ使いやすいよう、既定でUnlitです。ライトを適用したい場合は `unlit = false` の `Material3D` を明示的に渡します。
- UV は各形状で生成します。箱は面ごと、球・カプセルは経緯度、円柱・円錐は側面と底面を分離しています。`uvScale` と `uvOffset` で調整し、リピート＋線形ミップフィルタで描画します。
- BMP・PNG・JPEG などの WIC 対応画像、DDS・TGA・HDR を読み込めます。単一2Dテクスチャ／単一キューブマップが対象です。キューブマップは読み込めますが、現在の `Material3D` の各テクスチャには指定できません。
- 色画像は既定で sRGB、法線マップは `TextureLoadDesc::NormalMap()`、Metallic・Roughness・AOなどのデータ画像は `TextureLoadDesc::Data()` でLinearとして読み込みます。モデルに頂点タンジェントがある場合はhandednessを含めて使用し、持たないモデルや既存プリミティブではワールド位置とUVの画面微分からTBNを生成します。UVが退化してTBNを作れない箇所は頂点法線へフォールバックします。
- ミップ生成は既定で有効です。DDS などの既存ミップは保持し、`generateMipMaps = false` では新規生成しません。
- パスは UTF-8、相対パスは作業ディレクトリ基準です。通常は `Project` を作業ディレクトリにします。同じ正規化パスと読み込み設定はキャッシュし、GPU リソースはフレームワーク終了まで保持します。
- 初回ロードはGPU転送完了を待つ同期処理です。毎フレームのロードは避けてください。失敗時は例外を送出します。
- 異なるテクスチャを同じ形状のまとめ描画で扱うため、Shader Model 6.6 と Resource Binding Tier 3 が必要です。起動時に対応状況を確認します。

### HDRからDDSへの変換

同梱しているMicrosoft DirectXTex公式の`Texconv`プロジェクトをビルドし、`.hdr`ファイルを`Project/Tools/Texconv/ConvertHDRToDDS.bat`へドラッグ＆ドロップすると、同じ場所へLinearな`R16G16B16A16_FLOAT`のDDSを生成できます。複数ファイルにも対応し、既存DDSは既定でスキップします。

生成物はフルMip付きの正距円筒図法2Dテクスチャです。IBL用のDiffuse Irradiance Cubemap、GGX Specular Prefilter Cubemap、BRDF LUTは、最初にSkyboxを設定したフレームでGPU生成されます。

### Skybox

正距円筒図法のLinear HDR DDSを`Skybox`へ読み込み、描画するフレーム内で`SetScene()`を呼びます。背景はHDRの`SceneColor`へ出力されるため、ExposureとACESトーンマッピングが適用されます。

```cpp
Skybox skybox("Assets/Textures/Environment/StoryStudio02/story_studio_02_2k.dds");
skybox.intensity = 1.0f;
skybox.rotation = 0.0f;

while (System::Update()) {
    skybox.SetScene();
}
```

`intensity`は背景とIBLのHDR輝度へ乗算する係数、`rotation`は背景と環境反射に適用するY軸周りのラジアン角です。同じ環境テクスチャのIBL結果はキャッシュされ、毎フレーム再生成されません。

Metallic-Roughness PBRの間接光にはSplit-Sum近似を使用します。Diffuse Irradianceは拡散反射、RoughnessごとのGGX Prefilterは鏡面反射、BRDF LUTは視線角度とRoughnessに依存するBRDF積分を担当します。Skyboxを設定しないフレームは従来の最低限の環境光へフォールバックします。

### シャドウマップ

現在の平行光源は、モデルと面を持つ3Dプリミティブを2048x2048のD32シャドウマップへ描画し、直接光へ影を適用します。シーン側で特別な描画関数を呼ぶ必要はなく、通常の`Draw()`で登録した形状が自動的に影を落とします。`Line3D`とメシュレット境界球などのデバッグ描画は影を落としません。

現段階は原点周辺を覆う単一の正射影シャドウマップとハードシャドウです。カメラ追従、描画範囲・Biasの公開設定、PCF、カスケードシャドウマップは今後の拡張対象です。

### 静的モデル描画

`Model` はAssimp経由で静的モデルを同期ロードします。現在は三角形メッシュ、頂点位置、法線、タンジェント、UV、ベースカラー、ベースカラー・法線・Metallic-Roughness・AOテクスチャに対応しています。

```cpp
Model model("Assets/Models/BoxTextured/BoxTextured.glb");

while (System::Update()) {
    Camera3D(Vector3(0.0f, 1.0f, -5.0f)).SetScene();
    model.Draw(Math::Identity4x4());
}
```

モデルはDirectXMeshでメシュレットへ分割され、Amplification Shaderで現在の3Dカメラの視錐台に入るメシュレットだけを描画します。別のカメラでカリングしたい場合は、描画するフレーム内で `SetCullingScene()` を呼びます。

```cpp
Camera3D renderCamera({ 0.0f, 1.0f, -5.0f });
Camera3D cullingCamera({ 0.0f, 5.0f, -10.0f }, 0.0f, 20.0f * Math::DEG_TO_RAD);

ModelDrawOptions drawOptions{
    .enableNormalMap = true,
    .normalScale = 1.0f,
    .metallicScale = 1.0f,
    .roughnessScale = 1.0f,
    .occlusionStrength = 1.0f,
    .unlit = false,
    .debugDraw = {
        .showMeshletColors = true,
        .showMeshletBoundingSpheres = true,
        .meshletBoundingSphereColor = Color::Lime,
    },
};

while (System::Update()) {
    renderCamera.SetScene();
    cullingCamera.SetCullingScene();
    cullingCamera.DrawDebug({
        .showFrustum = true,
        .frustumColor = Color::Yellow,
    });
    model.Draw(Math::Identity4x4(), drawOptions);
}
```

`SetCullingScene()` を呼ばなかったフレームは描画カメラが自動的に使われます。メシュレット境界球はカリング結果に関係なくデバッグ表示されるため、別カメラでカリングした結果も描画カメラから確認できます。`Camera3DDebugDraw::showFrustum` を切り替えると、カメラの視錐台を示すラインの表示をシーン側からON/OFFできます。

同じモデルを複数描画する場合は、インスタンス配列をまとめて送れます。通常の `Draw()` を同じモデルへ複数回呼んだ場合も、内部では同じバッチへまとめられます。

```cpp
std::array<ModelInstance3D, 5> instances{};
for (size_t i = 0; i < instances.size(); ++i) {
    instances[i].worldMatrix = Math::MakeTranslateMatrix(
        { static_cast<float>(i) * 2.0f, 0.0f, 0.0f });
}
model.DrawInstanced(instances, drawOptions);
```

- GLB内の圧縮された埋め込み画像と、モデルから相対参照される外部画像を読み込めます。
- 読み込み時に左手座標系へ変換し、ノード変換を頂点へ適用します。glTFのベースカラー、タンジェント空間法線、Metallic-Roughness、AOのテクスチャと各係数はマテリアルから自動的に読み込みます。現時点ではスキニング、アニメーション、透過、IBLは対象外です。
- `ModelDrawOptions::enableNormalMap` を `false` にすると描画単位で法線マップを無効化できます。`normalScale`・`metallicScale`・`roughnessScale`・`occlusionStrength` はモデルファイル側の値へ乗算され、`color` は全インスタンス共通の色倍率として使われます。`unlit = true` ではモデルもライト計算を行いません。
- モデルは初回のみキャッシュされ、GPUリソースはフレームワーク終了まで保持されます。毎フレーム生成しないでください。

### 平行光源

`DirectionalLight` で光の進む方向・色・強度を指定し、描画するフレーム内で `SetScene()` を呼びます。

```cpp
DirectionalLight light({ 1.0f, -1.0f, 0.5f }, Color::White, 1.5f);

while (System::Update()) {
    Camera3D(Vector3(0.0f, 1.0f, -5.0f)).SetScene();
    light.SetScene();
    Box3D().Draw();
}
```

- `direction` は光が進む向きで、内部で正規化されます。ゼロベクトルの場合は既定の下向き光源を使用します。
- `intensity` は `0` 以上として扱い、負の値は `0` に補正します。

# Lazieal Game Engine

・シーンマネージャの実装  
・トランスフォームマネージャの実装  
・Line,Plane,Sphere,AABB,OBB,Capsuleの衝突判定関数を実装  
・Sphere,AABB,OBB,の物理コライダーを実装(重力、押し戻し、反発)
・Blenderのアドオンを使ってシーンを作成でき、ホットリロードができる  
・Effekseerで作成したエフェクトをインポートでき、ホットリロードできる  
・Figmaで作成したUIをインポートでき、ホットリロードできる  
・外部パラメータを変更できるexeを別途用意し、ImGuiで動的にパラメータを変更できるようにする
