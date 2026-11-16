# Post Processシステム仕様

## 概要

Post Process Profileは、適用するEffectと順序、初期パラメータをJsonで定義する。
Profileを読み込むと実行時インスタンスを指す世代付きハンドルが返り、ゲーム中の値は
ハンドル、Effect Id、Parameter名を指定して上書きできる。

標準の処理順は次のとおり。

```text
SceneColor HDR
  -> BeforeToneMapping
  -> ToneMapping（必須かつ1つ）
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
- `Stage`: `BeforeToneMapping`、`ToneMapping`、`AfterToneMapping`のいずれか。
- `Pipeline`: Pipeline Jsonの`Name`。Shaderパスではない。
- `Slot`: `0`から`7`までの固定`float4`スロット。
- `Components`: `X`、`XY`、`XYZ`、`XYZW`など、連続した成分を指定する。
- 同一スロット内でParameterの成分を重複させてはならない。

## Profile定義

`Assets/JsonData/PostProcess`以下へ配置する。EffectはStage順に並べ、ToneMappingを
有効な状態でちょうど1つ含める。

```json
{
  "Name": "Default",
  "Effects": [
    { "Id": "Bloom", "Effect": "Bloom", "Enabled": true },
    { "Id": "Output", "Effect": "ToneMapping" },
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
```

Profileからの相対パスを含まないファイル名は`Assets/JsonData/PostProcess`から解決する。
上書き値はEffect定義の型と範囲を満たす必要があり、不正な指定では`false`を返す。
Profile破棄時は`RemovePostProcessProfile`を呼ぶ。

## 初期実装

- Bloom: ToneMapping前。抽出、半解像度の水平・垂直Blur、HDR合成を行う専用Executor。
- Color Adjustment: ToneMapping後。Brightness、Contrast、Saturation、Tintを調整する。
- Vignette: ToneMapping後。画面周辺を指定色へ寄せる。
- Tone Mapping: HDRとLDRの境界として常に実行する。

