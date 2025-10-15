// 定数バッファの構造体 (C++側のConstant Bufferと対応させる)
cbuffer cb0 : register(b0)
{
    matrix viewProjection; // ビュープロジェクション行列
    float3 cameraRight; // カメラの右ベクトル (ビルボード計算に利用)
    float3 cameraUp; // カメラの上ベクトル (ビルボード計算に利用)
};

// テクスチャ (TextureManagerなどで設定されるSRV)
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

// 頂点シェーダーからピクセルシェーダーへの出力
struct VSOutput
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};