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
・スペキュラーマップの実装  


### テクスチャ付き3D描画

`Main()` 内でテクスチャを読み込み、`Material3D` を `Draw()` に渡します。

```cpp
Material3D material{};
material.baseColorTexture = Texture("Assets/Textures/Checker.bmp");
material.color = Color::White;
material.uvScale = { 4.0f, 4.0f };

while (System::Update()) {
    Camera3D(Vector3(0.0f, 1.0f, -5.0f)).SetScene();
    Box3D().Draw(Math::Identity4x4(), material);
}
```

- `Triangle3D`・`Box3D`・`Plane3D`・`Ground3D`・`Sphere3D`・`Capsule3D`・`Cone3D`・`Cylinder3D`・`Line3D` に対応しています。従来の色指定 `Draw(matrix, color)` も使用できます。
- `baseColorTexture` 未指定時は単色、指定時はテクスチャと `color` を乗算します。今回は不透明なベースカラー描画が対象で、透過・法線マップ・PBR は未実装です。
- UV は各形状で生成します。箱は面ごと、球・カプセルは経緯度、円柱・円錐は側面と底面を分離しています。`uvScale` と `uvOffset` で調整し、リピート＋線形ミップフィルタで描画します。
- BMP・PNG・JPEG などの WIC 対応画像、DDS・TGA・HDR を読み込めます。単一2Dテクスチャ／単一キューブマップが対象です。キューブマップは読み込めますが、`baseColorTexture` には指定できません。
- 色画像は既定で sRGB、データ用画像は `TextureLoadDesc::NormalMap()` で Linear として読み込めます。これは読み込み設定であり、法線マップ描画を有効にするものではありません。
- ミップ生成は既定で有効です。DDS などの既存ミップは保持し、`generateMipMaps = false` では新規生成しません。
- パスは UTF-8、相対パスは作業ディレクトリ基準です。通常は `Project` を作業ディレクトリにします。同じ正規化パスと読み込み設定はキャッシュし、GPU リソースはフレームワーク終了まで保持します。
- 初回ロードはGPU転送完了を待つ同期処理です。毎フレームのロードは避けてください。失敗時は例外を送出します。
- 異なるテクスチャを同じ形状のまとめ描画で扱うため、Shader Model 6.6 と Resource Binding Tier 3 が必要です。起動時に対応状況を確認します。

### 静的モデル描画

`Model` はAssimp経由で静的モデルを同期ロードします。現在は三角形メッシュ、頂点位置、法線、UV、ベースカラー、ベースカラーテクスチャに対応しています。

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

ModelDebugDraw debugDraw{
    .showMeshletColors = true,
    .showMeshletBoundingSpheres = true,
    .meshletBoundingSphereColor = Color::Lime,
};

while (System::Update()) {
    renderCamera.SetScene();
    cullingCamera.SetCullingScene();
    cullingCamera.DrawDebug({
        .showFrustum = true,
        .frustumColor = Color::Yellow,
    });
    model.Draw(Math::Identity4x4(), Color::White, debugDraw);
}
```

`SetCullingScene()` を呼ばなかったフレームは描画カメラが自動的に使われます。メシュレット境界球はカリング結果に関係なくデバッグ表示されるため、別カメラでカリングした結果も描画カメラから確認できます。`Camera3DDebugDraw::showFrustum` を切り替えると、カメラの視錐台を示すラインの表示をシーン側からON/OFFできます。

- GLB内の圧縮された埋め込み画像と、モデルから相対参照される外部画像を読み込めます。
- 読み込み時に左手座標系へ変換し、ノード変換を頂点へ適用します。現時点ではスキニング、アニメーション、法線マップ、PBRマテリアル描画は対象外です。
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
