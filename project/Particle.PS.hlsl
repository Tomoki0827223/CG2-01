#include "Particle.hlsl"

float4 main(VSOutput input) : SV_TARGET
{
    // テクスチャサンプリング
    float4 texColor = gTexture.Sample(gSampler, input.uv);
    
    // テクスチャのアルファ値と頂点シェーダからのアルファ値を乗算
    float4 finalColor = texColor * input.color;

    // 加算ブレンド (Additive Blending) を使用するため、テクスチャのRGBに頂点カラーを乗算
    // 加算ブレンドの場合、最終的なアルファ値は1.0に固定する設定にすることも多い
    // 今回は頂点シェーダーからのアルファ値をそのまま使用します。
    return finalColor;
}