# レンダリング機能のパス分類規約

## 目的

本書は、新しい描画機能を追加するときに、その処理をLighting、Scene Composition、
Post Processのどこへ配置するかを判断するための仕様である。

実装方式がフルスクリーン描画やCompute Shaderであることだけを理由に、処理を
Post Processへ分類してはならない。分類は、その処理がレンダリング上で表現する意味と、
前後の処理に対する依存関係によって決定する。

## 基本原則

描画機能は、次の3種類へ分類する。

1. **Lighting Input / Lighting**
   - 光が物体表面でどのように見えるかを決定する処理。
2. **Scene Composition**
   - ライティング済みのシーンへ、空間中の媒質、空、反射などを合成する処理。
3. **Post Process**
   - 完成したシーン画像の見せ方を加工する処理。

判断時は、次の規則を優先する。

> 処理結果を後続のライティングが参照する必要がある場合はLighting InputまたはLighting、
> カメラと表面の間に存在する空間や媒質を表現する場合はScene Composition、
> 完成画像の見た目だけを変える場合はPost Processへ配置する。

G-BufferやDepthを参照するかどうかは、分類を決める直接の条件ではない。
DepthやNormalを補助入力として使用していても、完成画像の見た目だけを変更する処理は
Post Processに分類できる。

## 標準描画順序

標準的な描画順序を次に示す。

```text
G-Buffer Pass
  ↓
Lighting Input Passes
  ├─ SSAO
  ├─ Screen Space Shadow
  └─ Reflection用中間情報
  ↓
Deferred Lighting
  ├─ Direct Lighting
  ├─ Indirect Lighting
  ├─ Shadow
  ├─ IBL
  └─ Reflection
  ↓
Scene Composition
  ├─ Sky
  ├─ Fog
  ├─ Volumetric Lighting
  ├─ Volumetric Light Shaft
  └─ SSRの最終合成
  ↓
Transparent Forward Lighting
  └─ FogやVolumetric情報を参照
  ↓
HDR Post Process
  ├─ Bloom
  ├─ Depth of Field
  └─ Motion Blur
  ↓
Tone Mapping
  ↓
LDR Post Process
  ├─ Vignette
  ├─ Color Adjustment
  ├─ Film Grain
  └─ FXAA
  ↓
2D / ImGui
```

実装の都合で一部の処理を統合する場合でも、この論理的な順序と依存方向を維持する。

## Lighting Input / Lighting

### 分類条件

次のいずれかに該当する処理は、Lighting InputまたはLightingへ配置する。

- BRDFの入力または計算結果を変更する。
- 光源の寄与を計算する。
- ShadowやVisibilityを計算する。
- Ambient LightやIndirect Lightを変化させる。
- Reflectionをライティング結果として加える。
- 処理結果を後続のライティングから参照する。

Lighting Inputはライティング用の中間情報を生成し、Lightingはその情報を使用して
HDR SceneColorを生成する。

### リソースアクセス

```text
Lighting Input
  読み取り: G-Buffer、Depth、Shadow、Camera
  書き込み: AO、Reflection、VisibilityなどのLighting用中間Texture

Lighting
  読み取り: G-Buffer、Depth、Shadow、Lighting用中間Texture
  書き込み: HDR SceneColor
```

### 代表例

| 機能 | 配置 | 理由 |
| --- | --- | --- |
| Directional / Point Light | Lighting | 光源の寄与を計算する。 |
| Toon Lighting | Lighting | 表面に対する光の寄与を段階化する。 |
| Shadow Map | Lighting | 光源からのVisibilityを決定する。 |
| IBL | Lighting | 表面の間接光と反射を計算する。 |
| SSAO | Lighting Input | Ambient Lightへ適用する遮蔽率を生成する。 |
| Screen Space Shadow | Lighting Input | Lightingが使用するVisibilityを生成する。 |
| Decal | G-BufferまたはLighting前 | 表面の材質情報を変更する。 |

SSAOを最終画像へ黒色として乗算してはならない。その方法ではDirect LightやEmissiveまで
暗くなるため、AO Textureを生成してAmbient / Indirect Lightingへ適用する。

## Scene Composition

### 分類条件

次のいずれかに該当する処理はScene Compositionへ配置する。

- カメラと物体表面の間に存在する媒質を表現する。
- 空間中での光の吸収、透過、散乱を表現する。
- ライティング済みSceneColorへ空や反射を合成する。
- Transparent描画からも同じ空間情報を参照する必要がある。

Scene CompositionはLightingとPost Processの間に存在する独立した工程であり、
単にフルスクリーンで実装されているという理由でPost Processへ含めない。

### リソースアクセス

```text
読み取り:
  HDR SceneColor
  Depth
  G-Buffer
  Camera
  Shadow
  Volumetric Texture

書き込み:
  HDR SceneColorまたはComposition用HDR Texture
```

### 代表例

| 機能 | 配置 | 理由 |
| --- | --- | --- |
| Distance / Height Fog | Scene Composition | カメラと表面の間の媒質を表現する。 |
| Volumetric Fog | Scene Composition | 空間中の光の吸収と散乱を計算する。 |
| Volumetric Light Shaft | Scene Composition | Shadowを伴う空間的な光の散乱である。 |
| Sky Atmosphere | Scene Composition | シーンの背景と大気散乱を合成する。 |
| SSR | LightingまたはScene Composition | 反射情報を生成し、ライティング済みシーンへ合成する。 |

Fogを実装する場合、Opaque SceneColorへ合成した後、Transparent ShaderからもFogまたは
積分済みVolumetric Textureを参照できる構成を基本とする。

```text
Opaque Lighting
  ↓
Fog / Volumetric Composition
  ↓
Transparent Forward Lighting
  └─ 同じFog情報を参照
```

Depthだけを使用してTransparent描画後にFogを重ねる方法は、簡易表現としてのみ扱う。
この方法では、透明物に対して背景のDepthが使用されるため、標準のFog実装には使用しない。

## Post Process

### 分類条件

次の条件を満たす処理はPost Processへ配置する。

- 主な入力が完成済みのSceneColorである。
- 処理結果をLightingから参照しない。
- G-Buffer、Depth、Normalなどは補助入力として読み取るだけである。
- G-Buffer、Depth、Shadow、Lighting用中間Textureを書き換えない。
- カメラ表現、画作り、表示上の演出を目的とする。

### リソースアクセス

```text
読み取り:
  現在のColor
  元のHDR SceneColor
  Depth
  Normal
  Albedo
  Material
  Camera

書き込み:
  Post Process用Ping-Pong Colorのみ
```

Post Effectは、G-Buffer、Depth、Shadow、Lighting用中間Textureを読み取ってもよいが、
書き換えてはならない。また、Post Effectの出力をLightingから参照してはならない。

### HDR / LDRの分類

Post ProcessはTone Mappingを境界として、HDRとLDRに分ける。

#### HDR Post Process

HDRの光量情報を必要とする処理はTone Mapping前へ配置する。

- Bloom
- Exposure
- HDR Color Grading
- Depth of Field
- Motion Blur
- Lens Flare

#### Tone Mapping

Tone MappingはHDR SceneColorから表示可能なLDR Colorへの必須変換として扱う。
無効化可能な演出用Post Effectとして扱ってはならない。

#### LDR Post Process

ディスプレイ上の最終的な見せ方を変更する処理はTone Mapping後へ配置する。

- Vignette
- Color Adjustment
- Grayscale
- Posterization
- Sepia
- Color Inversion
- Lens Distortion
- Radial Blur
- Sharpen
- Film Grain
- Pixelation
- Ordered Dithering
- Chromatic Aberration
- FXAA

### Scene Textureを使うPost Effect

DepthやNormalを使用していても、結果が完成画像に対する演出であり、Lightingへ影響しない場合は
Post Processに分類する。例えば、DepthとNormalから輪郭を検出して色を重ねる処理は
Post Processである。

Scene TextureはPost Effectから読み取り専用で使用する。

```text
gInputColor     直前のPost Effect結果
gSceneColorHDR  Post Process開始前のHDR SceneColor
gSceneDepth     Opaque GeometryのDepth
gSceneNormal    Opaque GeometryのWorld Normal
gSceneAlbedo    Opaque GeometryのAlbedo
gSceneMaterial  Opaque GeometryのMaterial情報
```

Opaque G-Buffer由来のDepth、Normal、Albedo、Materialには、原則として透明物の情報が
含まれない。この制約を使用するエフェクトの仕様へ明記する。

## 近似表現の命名規則

物理的な現象を画面空間の近似表現として実装する場合は、本来の機能と区別できる名前を付ける。

| 本来の機能 | 近似表現 |
| --- | --- |
| `VolumetricLightShaft` | `RadialLightShaft` |
| `VolumetricFog` | `ScreenFogOverlay` |

例えば、太陽位置へ向けてSceneColorを放射ブラーする処理は`RadialLightShaft`として
Post Processへ配置する。ShadowとVolumetric Fogによって空間的な散乱を計算する処理は
`VolumetricLightShaft`としてScene Compositionへ配置する。

近似表現へ本来の機能と同じ名前を付けてはならない。ユーザーが品質、制限、処理負荷を
名前から判断できる状態を維持する。

## 判断フロー

新しい描画機能を追加するときは、次の順序で分類する。

```text
Q1. 光源、BRDF、Shadow、Ambient、Reflectionの計算に影響するか？
  Yes → Lighting Input / Lighting
  No  ↓

Q2. カメラと表面の間にある媒質や、空間的な光を表現するか？
  Yes → Scene Composition
  No  ↓

Q3. 完成した画像の見せ方だけを変更するか？
  Yes → Post Process
  No  → 描画フローと責務を個別に再検討する

Q4. Post Processの場合、HDRの光量情報が必要か？
  Yes → HDR Post Process / Before Tone Mapping
  No  → LDR Post Process / After Tone Mapping
```

## 代表機能の分類一覧

| 機能 | 分類 |
| --- | --- |
| SSAO | Lighting Input |
| Screen Space Shadow | Lighting Input |
| Deferred Direct / Point Lighting | Lighting |
| Shadow | Lighting |
| IBL | Lighting |
| SSR | Lighting / Scene Composition |
| Distance / Height Fog | Scene Composition |
| Volumetric Fog | Scene Composition |
| Volumetric Light Shaft | Scene Composition |
| Sky Atmosphere | Scene Composition |
| Radial Light Shaft | HDR Post Process |
| Screen Fog Overlay | HDR Post Process |
| Bloom | HDR Post Process |
| Depth of Field | HDR Post Process |
| Motion Blur | HDR Post Process |
| Tone Mapping | HDRからLDRへの必須変換 |
| Vignette | LDR Post Process |
| Color Adjustment | LDR Post Process |
| Film Grain | LDR Post Process |
| FXAA | LDR Post Process |

## 実装レビュー時の確認項目

新しい描画機能を追加またはレビューするときは、次の項目を確認する。

- 機能を実装方法ではなく、レンダリング上の意味で分類したか。
- 後続のLightingが結果を必要としないか。
- カメラと表面の間の媒質をPost Effectとして処理していないか。
- G-Buffer、Depth、ShadowをPost Effectから書き換えていないか。
- HDR情報が必要な処理をTone Mapping後へ配置していないか。
- LDR前提の処理をHDR値へ直接適用していないか。
- Transparent描画がFogやVolumetric情報を正しく参照できるか。
- 画面空間の近似表現を、本来の物理表現と区別できる名前にしたか。
- Scene Textureに透明物が含まれない制約を明記したか。

判断に迷う場合は、機能の入力と出力、それを参照する後続処理を先に整理する。
分類を曖昧にしたまま既存のPost Processへ追加してはならない。
