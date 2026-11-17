# Post Processシステム仕様

## 概要

Post Process Profileは、適用するEffectと順序、初期パラメータをJsonで定義する。
Profileを読み込むと実行時インスタンスを指す世代付きハンドルが返り、ゲーム中の値は
ハンドル、Effect Id、Parameter名を指定して上書きできる。

標準の処理順は次のとおり。

```text
SceneColor HDR
  -> BeforeToneMapping
  -> ToneMapping（必須の出力変換）
  -> AfterToneMapping
  -> SwapChain
  -> 2D / ImGui
```

中間結果はHDR用とLDR用のRenderTextureをそれぞれピンポンして保持する。

## Post Effect定義

`Assets/JsonData/PostEffect`以下のJsonを起動時に読み込む。

```json
{
  "Name": "MyEffect",
  "Executor": "Fullscreen",
  "Pipeline": "PostProcessMyEffect",
  "Stage": "AfterToneMapping",
  "Parameters": {
    "Intensity": {
      "Slot": 0,
      "Components": "X",
      "Default": 1.0,
      "Minimum": 0.0,
      "Maximum": 1.0
    }
  }
}
```

- `Executor`: 汎用Effectは`Fullscreen`を使う。複数Passなど固有処理が必要なEffectは専用Executorを実装する。
- `Stage`: `BeforeToneMapping`または`AfterToneMapping`。Tone Mapping自体は通常Effectとして定義しない。
- `Pipeline`: Pipeline Jsonの`Name`。Shaderパスではない。
- `Slot`: `0`から`7`までの固定`float4`スロット。
- `Components`: `X`、`XY`、`XYZ`、`XYZW`など、連続した成分を指定する。
- 同一スロット内でParameterの成分を重複させてはならない。

## Tone Mapping定義

`Assets/JsonData/ToneMapping`以下のJsonを起動時に読み込む。Tone Mappingは自由に
追加・無効化するEffectではなく、HDRとLDRの間に必ず実行する出力変換として扱う。

```json
{
  "Name": "ACES",
  "Pipeline": "PostProcessToneMapping",
  "Parameters": {
    "ExposureEV": {
      "Slot": 0,
      "Components": "X",
      "Default": 0.0,
      "Minimum": -10.0,
      "Maximum": 10.0
    }
  }
}
```

Profileは`Method`で使用するTone Mapping定義を選択する。将来ReinhardやNeutralなどを
追加する場合も、通常Effect一覧へ混ぜずにこの定義を追加する。

## Profile定義

`Assets/JsonData/PostProcess`以下へ配置する。Profileは必須の`ToneMapping`設定と、
任意の`Effects`から構成する。EffectはStage順に並べる。

```json
{
  "Name": "Default",
  "ToneMapping": {
    "Method": "ACES",
    "Parameters": {
      "ExposureEV": 0.0
    }
  },
  "Effects": [
    { "Id": "Bloom", "Effect": "Bloom", "Enabled": true },
    { "Id": "Color", "Effect": "ColorAdjustment", "Enabled": true }
  ]
}
```

`Id`はProfile内の実行時インスタンス名、`Effect`はPost Effect定義の`Name`である。
同じEffect定義を異なるIdと値で複数回使用できる。

## Shaderインターフェース

汎用Fullscreen Effectは必要なリソースだけを宣言する。PipelineのShader Reflectionに
存在する予約名だけが自動でバインドされるため、色だけを使うEffectとDepthやNormalを
使うEffectを同じ仕組みで追加できる。

| 予約名 | 内容 |
| --- | --- |
| `gInputColor` | 直前のEffect結果 |
| `gSceneColorHDR` | Post Process開始前のHDR SceneColor |
| `gSceneDepth` | Scene Depth |
| `gSceneNormal` | G-Buffer Normal |
| `gSceneAlbedo` | G-Buffer Albedo |
| `gSceneMaterial` | G-Buffer Material |
| `PostProcessParameters` | `float4 gPostProcessParameters[8]` |
| `PostProcessContext` | 入出力・Sceneのサイズと逆数、Frame Index |

```hlsl
cbuffer PostProcessParameters : register(b0)
{
    float4 gPostProcessParameters[8];
};

cbuffer PostProcessContext : register(b1)
{
    float2 gOutputSize;
    float2 gInverseOutputSize;
    float2 gInputSize;
    float2 gInverseInputSize;
    float2 gSceneSize;
    float2 gInverseSceneSize;
    uint gFrameIndex;
    float3 gContextPadding;
};
```

Samplerは名前に`Point`を含むとPoint Filter、`Clamp`を含むとClamp Addressとなる。
たとえば`gLinearClampSampler`はLinear Clamp Samplerとして生成される。

## C++ API

```cpp
const auto profile = Graphics3D::CreatePostProcessProfile("Default.json");
Graphics3D::SetPostProcessProfile(profile);
Graphics3D::SetPostProcessEffectEnabled(profile, "MainBloom", true);
Graphics3D::SetPostProcessParameter(profile, "MainBloom", "Intensity", 0.25f);
Graphics3D::SetToneMappingParameter(profile, "ExposureEV", 1.0f);
```

Profileからの相対パスを含まないファイル名は`Assets/JsonData/PostProcess`から解決する。
上書き値はEffect定義の型と範囲を満たす必要があり、不正な指定では`false`を返す。
`ClearPostProcessOverrides`は通常EffectとTone Mappingの上書きをまとめてProfile初期値へ戻す。
Profile破棄時は`RemovePostProcessProfile`を呼ぶ。

## 初期実装

- Bloom: ToneMapping前。抽出、半解像度の水平・垂直Blur、HDR合成を行う専用Executor。
- Color Adjustment: ToneMapping後。Brightness、Contrast、Saturation、Tintを調整する。
- Grayscale: ToneMapping後。輝度色への変換量を調整する。
- Posterization: ToneMapping後。色成分の階調数を減らす。
- Sepia: ToneMapping後。セピア色への変換量を調整する。
- Color Inversion: ToneMapping後。反転色への変換量を調整する。
- Lens Distortion: ToneMapping後。指定した中心を基準にレンズ歪みを適用する。
- Radial Blur: ToneMapping後。指定した中心へ向かう放射状のBlurを適用する。
- Chromatic Aberration: ToneMapping後。画面中心からの距離に応じて色成分をずらす。
- Vignette: ToneMapping後。画面周辺を指定色へ寄せる。
- Sharpen: ToneMapping後。周囲との差が閾値以上の輪郭を強調する。
- FXAA: ToneMapping後。輝度差からエッジを検出してアンチエイリアスを適用する。
- Pixelation: ToneMapping後。指定したピクセル幅のブロック単位で表示する。
- Ordered Dithering: ToneMapping後。Bayer Matrixを使って指定した色階調へ量子化する。
- Tone Mapping: 通常Effectとは分離し、Profileが選択した方式をHDRとLDRの境界で常に実行する。

