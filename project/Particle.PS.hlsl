// resources/shaders/Particle.PS.hlsl

// 頂点シェーダーからの入力構造体
struct VSOutput
{
    float4 svpos : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

// テクスチャサンプラーとテクスチャリソース
SamplerState sampler0 : register(s0);
Texture2D<float4> texture0 : register(t0);

// ピクセルシェーダーのエントリーポイント
float4 main(VSOutput input) : SV_TARGET
{
    // 1. テクスチャをサンプリング
    float4 textureColor = texture0.Sample(sampler0, input.uv);

    // 2. パーティクルの色とテクスチャの色を乗算
    float4 finalColor = input.color * textureColor;

    // 3. アルファテスト/ブレンドの調整
    // (通常、パーティクルは半透明なのでアルファブレンドを有効にする必要があります)
    
    return finalColor;
}