# Texconv HDR to DDS

同梱しているMicrosoft DirectXTex公式の`texconv`を使い、Radiance `.hdr`画像を同じフォルダーへLinearな`R16G16B16A16_FLOAT`の`.dds`として保存します。フルMipチェーンも生成します。

## 使い方

1. Visual Studioで`Texconv`プロジェクトをビルドします。
2. 1枚または複数枚の`.hdr`を`ConvertHDRToDDS.bat`へドラッグ＆ドロップします。
3. 入力画像と同じ場所に、同名の`.dds`が生成されます。

バッチは`Develop`、`Release`、`Debug`の順にビルド済み`texconv.exe`を探し、見つからなければ`PATH`上の`texconv.exe`を使います。既存のDDSはスキップします。

上書きしたい場合は`texconv`へ`-y`を追加して直接実行します。

```powershell
texconv.exe -y -f R16G16B16A16_FLOAT -m 0 -dx10 -o Assets/Textures/Environment Assets/Textures/Environment/example.hdr
```

出力は正距円筒図法の2D DDSです。Cubemap変換、Diffuse Irradiance、Specular PrefilterなどのIBL前処理は行いません。

オプションの詳細は[DirectXTex texconv wiki](https://github.com/microsoft/DirectXTex/wiki/Texconv)を参照してください。
