# Lazieal Graphics Framework TODO

現在の実装を基準に、描画フレームワークとして伸ばす項目を優先度順に整理しています。

凡例:

- **P0**: 先に解消しないと、以降の機能・性能・使いやすさへ大きく影響する基盤
- **P1**: 描画フレームワークとして次に欲しい主要機能
- **P2**: 品質、制作効率、対応範囲を広げる機能
- **Research**: 基盤が整ってから試す先進機能

## 実装済みの基盤

以下はすでに動く構成が存在するため、Todoではなく今後の拡張元として扱います。

- [x] Mesh Shaderによる2D / 3Dプリミティブ描画
- [x] Deferred Rendering用のAlbedo / Normal / Material G-Buffer
- [x] Metallic-Roughness PBRと法線・AOマッピング
- [x] HDRシーンカラー、EV露出補正、ACES Tone Mapping
- [x] SkyboxとSplit-Sum IBL
- [x] 平行光源
- [x] 4分割CSM、境界ブレンド、Texel Snapping、3 x 3 PCF
- [x] Assimp + DirectXMeshによる静的モデルとメッシュレット生成
- [x] Amplification Shaderによるメッシュレット視錐台カリング
- [x] モデルのインスタンスバッチ
- [x] JSONベースのGraphics / Compute Pipeline生成
- [x] テクスチャキャッシュ、Mip生成、sRGB / Linear指定
- [x] ウィンドウリサイズ追従
- [x] ImGui、キーボード、マウス、Raw Input

## P0: フレーム実行とGPUリソースの土台

### CPU / GPUの並列化

- [ ] 2～3フレーム分のFrame Contextを導入する
  - Command Allocator
  - フレームごとのFence値
  - Dynamic Upload Buffer / Constant Buffer
  - フレームごとの一時Descriptor領域
- [ ] `Fence::EndFrame()` の毎フレーム完全待機を廃止する
- [ ] Fence Eventを毎回生成せず、ライフタイム中再利用する
- [ ] Swap ChainのBack Buffer数とFrame Contextを対応付ける
- [ ] GPUタイムアウト・Device Removed時に診断情報を残す

現状は毎フレーム `WaitForSingleObject(..., INFINITE)` まで待つため、CPUとGPUが直列化されています。新しい描画機能より先に直す価値が最も高い部分です。

### GPUメモリとアップロード

- [ ] 静的モデルの頂点・メッシュレットデータをDefault Heapへ移す
- [ ] 共通Upload Ring BufferとCopy Queueを設計する
- [ ] テクスチャ・モデルの非同期ロードAPIを追加する
- [ ] ロード完了前に使えるPlaceholder Texture / Modelを用意する
- [ ] Upload Bufferと中間リソースをFence完了後に回収する
- [ ] Texture / Modelの明示解放または参照数ベースの回収を実装する
- [ ] Descriptorの解放・再利用と枯渇時の安全な処理を実装する

### ライブラリとしての境界

- [ ] フレームワーク本体をStatic LibraryまたはDLLへ分離する
- [ ] Sandbox / Exampleアプリを別ターゲットへ分離する
- [ ] 公開ヘッダーが `Project/Src` の内部型へ依存しない状態を維持する
- [ ] 利用側プロジェクトからLink・Include・Assets配置を再現できるPremake設定を用意する
- [ ] サードパーティのバージョン、ライセンス、更新方法を文書化する
- [ ] 最小サンプル、PBRサンプル、モデル大量描画サンプルを分離する

現状は `LaziealGraphicsFramework` 自体が `WindowedApp` で、`main.cpp` と内部実装を同時にビルドします。再利用可能なフレームワークを目指すなら、この境界整理が必須です。

### 失敗処理と検証

- [ ] DirectX APIの失敗を `assert` だけに依存せず、Releaseでも内容を返す
- [ ] HRESULT、ファイルパス、Pipeline名、Shader名を含む統一エラー型を作る
- [ ] 初期化途中で失敗した場合も安全に後始末できるようにする
- [ ] 数学、CSM分割、パス正規化、キャッシュキーの単体テストを追加する
- [ ] 全Pipeline JSONのSchema検証を追加する
- [ ] 全HLSLのオフラインコンパイルを自動化する
- [ ] 最小の描画スモークテストとGPU Validation実行手順を用意する

## P1: レンダリングアーキテクチャ

### Render Graphとリソース状態管理

- [ ] Render Pass間の依存関係を表すRender Graphを導入する
- [ ] Resource Barrier、Render Target寿命、一時Texture再利用を自動化する
- [ ] Pass単位でGPU Markerを付け、PIXで追跡しやすくする
- [ ] Depth Pre-Passを選択可能にする
- [ ] Opaque / Alpha Mask / Transparent / Overlayの描画キューを分離する
- [ ] Forward+またはClustered Forward用の経路を追加し、透過物と多光源へ備える

現在は描画順が `GraphicsSystem::EndFrame()` に固定されています。ポストエフェクトや複数の描画経路を増やす前に、Passの依存関係をデータとして扱える形へ寄せると拡張しやすくなります。

### PipelineとShader管理

- [ ] Pipeline JSONへSchema Versionを追加する
- [ ] JSONの不正値・重複Pipeline名・不足Shader Stageを例外として報告する
- [ ] Shader Include依存を追跡したHot Reloadを実装する
- [ ] コンパイル済みShaderとPSO Libraryをディスクキャッシュする
- [ ] Shader ReflectionからRoot Parameterの整合性を検証する
- [ ] Root SignatureとStatic Samplerの重複を整理する
- [ ] Debug / DevelopでShaderの警告を見逃さない仕組みを作る

### GPU Driven Rendering

- [ ] CPU側のモデルバッチ作成コストを計測する
- [ ] Instance / Meshlet可視判定結果をGPU Bufferへ出力する
- [ ] ExecuteIndirectまたはGPU側Dispatch引数生成を検討する
- [ ] Occlusion Culling用のHi-Z Depth Pyramidを追加する
- [ ] メッシュレットのNormal Coneを可視判定に利用する
- [ ] LODをメッシュレットまたはモデル単位で選択する

現在のAmplification ShaderはFrustum Plane対Bounding Sphereの判定まで実装済みです。次の段階は、Normal Cone、Hi-Z Occlusion、LODを加えて「見えない仕事」をさらに減らすことです。

## P1: ライティングとマテリアル

### ライト

- [ ] Point Lightを追加する
- [ ] Spot Lightを追加する
- [ ] 複数ライトを扱うLight Bufferを設計する
- [ ] Tiled / Clustered Light Cullingを実装する
- [ ] 光源の物理単位と減衰モデルを定義する
- [ ] Directional Lightを複数扱う必要があるかAPI設計時に決める

### マテリアル

- [ ] Emissive Color / TextureをG-Bufferへ追加する
- [ ] Alpha Maskと両面マテリアルに対応する
- [ ] Transparent用のForward Passとソートを実装する
- [ ] glTF Sampler、UV Set、Texture Transformへ対応する
- [ ] glTFのMaterial Extension対応方針を決める
  - Clearcoat
  - Sheen
  - Transmission / Volume
  - Specular
  - IOR
- [ ] Anisotropy、Subsurface、Decalを必要性に応じて追加する

まずEmissive、Alpha Mask、Transparentを揃えると、現在読み込めるglTFの見た目を大きく改善できます。G-Bufferの空きチャンネルだけで無理に詰めず、帯域と将来拡張を測定して形式を決めます。

### シャドウ

- [ ] CSM設定を公開APIまたは外部設定へ出す
  - 最大距離
  - 分割数 / Split Lambda
  - 解像度
  - Depth Bias / Slope Scaled Bias
  - Cascade Blend幅
  - PCFカーネル
- [ ] Scene BoundsまたはCaster Boundsを使って不要な影描画を減らす
- [ ] Shadow CasterのAlpha Maskへ対応する
- [ ] Point / Spot Light用Shadow Mapを追加する
- [ ] PCSS、EVSMなどのソフトシャドウ方式を比較する
- [ ] Contact Shadowを必要性に応じて追加する
- [ ] Shadow AtlasまたはTexture Arrayの管理方式を整理する

## P1: ポストプロセスと画質

- [ ] Luminance Histogramを使ったAuto Exposure
- [ ] Bloom
- [ ] SSAOまたはGTAO
- [ ] TAAとMotion Vector
- [ ] FXAAなど軽量な代替AA
- [ ] Color Grading LUT
- [ ] Vignette、Chromatic Aberration、Film Grainを任意Pass化する
- [ ] Height Fog / Volumetric Fog
- [ ] Screen Space Reflection
- [ ] HDRディスプレイ出力（HDR10 / scRGB）の方針を決める
- [ ] Dynamic ResolutionまたはUpscalingの接続点を用意する

TAAを入れる場合は、Jitter、前フレーム行列、Motion Vector、履歴Texture、カメラカット判定をまとめて設計します。

## P1: 2DとUI

- [ ] `Sprite` と2Dテクスチャ描画を追加する
- [ ] UV切り出し、Pivot、反転、回転、色乗算に対応する
- [ ] Sprite Atlasと描画バッチを実装する
- [ ] Bitmap FontまたはSDF / MSDF Font描画を追加する
- [ ] Scissor / Clip Rectを公開する
- [ ] `BlendMode` を2D描画APIへ接続する
- [ ] 2D Cameraの変換を全2Dプリミティブへ一貫して適用する
- [ ] 2D座標系、DPI、ウィンドウリサイズ時の振る舞いを文書化する

初心者が使いやすいフレームワークという目標には、3Dの高度化と同じくらいSprite・文字・UIの短いAPIが重要です。

## P2: モデルとアセットパイプライン

- [ ] Skeletal AnimationとSkinning
- [ ] Animation Clip、Blend、State遷移
- [ ] Morph Target
- [ ] ノード階層を保持したTransform更新
- [ ] LODのインポートと生成
- [ ] KTX2 / Basis Universalを含む圧縮テクスチャ
- [ ] BCn圧縮とMip生成を含むオフライン変換
- [ ] Meshletを実行時ではなくビルド時に生成する独自アセット形式
- [ ] Texture / Material / Meshの重複排除
- [ ] ファイル変更監視とアセットHot Reload
- [ ] Asset ID、依存関係、Cook済み出力を管理するAsset Database
- [ ] 読み込み進捗、キャンセル、優先度を持つ非同期ロード

現在はAssimpの `aiProcess_PreTransformVertices` により静的な描画へ最適化しています。アニメーションやノード単位編集を入れる段階で、インポート表現を分離する必要があります。

## P2: APIと使いやすさ

- [ ] ウィンドウ名、初期サイズ、フルスクリーン、VSyncを利用側から設定可能にする
- [ ] MSAA、影、露出、描画品質をまとめたGraphics Settingsを追加する
- [ ] `Transform3D` を直接受け取る `Draw()` オーバーロードを検討する
- [ ] Camera、Light、Skyboxのフレーム既定値と上書き規則を文書化する
- [ ] 固定上限超過時にassertではなく安全な分割描画または明示エラーを行う
- [ ] Texture / Modelのロード状態、サイズ、メモリ使用量を取得可能にする
- [ ] ファイルパスAPIを `std::filesystem::path` とUTF-8で一貫させる
- [ ] Public APIへDoxygenコメントと最小サンプルを追加する
- [ ] API互換性ルールとVersioning方針を決める
- [ ] Headlessなアセット変換・テスト実行経路を用意する

## P2: デバッグ、計測、制作支援

- [ ] ImGuiでRenderer Statsを表示する
  - Draw / Dispatch数
  - Primitive / Instance / Meshlet数
  - Culling前後の数
  - Descriptor使用量
  - Texture / Bufferメモリ
- [ ] GPU Timestamp QueryによるPass別プロファイラを追加する
- [ ] G-Buffer、Depth、Cascade、Shadow Map、IBL Textureの可視化
- [ ] Wireframe、Overdraw、Mip Level、Normal / Roughness / Metallic表示
- [ ] PIX Event / Markerとオブジェクト名を整備する
- [ ] DREDとAftermath相当のクラッシュ診断方針を決める
- [ ] フレームキャプチャしやすい再現シーンを固定する
- [ ] Runtime Consoleと設定値編集を追加する

## P2: 堅牢性と性能の細部

- [ ] Resize時に必要なリソースだけ再生成する
- [ ] Minimize、Alt+Enter、モニター移動、DPI変更を検証する
- [ ] Present Mode、VSync、Tearing対応を選択可能にする
- [ ] Primitive Drawerの固定4096上限を動的Batchへ置き換える
- [ ] Modelの最大16384 Instance / 8192 Dispatch制約を統計とエラーへ露出する
- [ ] CPU側 `Inverse` / `Transpose` の重複計算を削減する
- [ ] PipelineとRoot Signatureの切り替え回数を計測・削減する
- [ ] Sampler管理を静的Sampler固定から拡張する
- [ ] Resource Binding Tier 3 / Mesh Shader非対応GPU向けFallbackの要否を決める

Fallbackを入れると対応GPUは増えますが、実装・検証コストも大きく増えます。高度な描画技術を優先する方針なら、要件を明示してMesh Shader専用を維持する判断も有効です。

## Research: 将来の高度な描画技術

- [ ] DirectX RaytracingによるReflection / Shadow / GI
- [ ] Hybrid RenderingとDenoiser
- [ ] Variable Rate Shading
- [ ] Sampler FeedbackとTexture Streaming
- [ ] Virtual Texturing
- [ ] Mesh ShaderベースのVirtualized Geometry
- [ ] Work Graphs
- [ ] ReSTIR DI / GI
- [ ] Neural Upscaling APIを差し込める抽象化

これらは最強だけど、Frame Context、GPUメモリ管理、Render Graph、計測基盤がない状態で先に入れると保守がかなり厳しくなります。まずP0とP1で「速く、壊れにくく、機能を足しやすい」土台を完成させてから着手します。

## 推奨実装順

1. Frame Context化と毎フレームGPU待機の解消
2. 本体ライブラリとSandboxの分離
3. Default Heap + 非同期アップロード + Descriptor再利用
4. 統一エラー処理、JSON / HLSL自動検証、GPUプロファイラ
5. Render Graphと描画キュー分離
6. Emissive / Alpha Mask / Transparent
7. 複数ライトとClustered Lighting
8. Sprite / Font / 2D Batch
9. Bloom / Auto Exposure / AO / TAA
10. Animation、LOD、Cook済みアセットパイプライン

この順なら、現在の高度な3D機能を壊しにくくしながら、性能・画質・初心者向けAPIをバランスよく伸ばせます。
