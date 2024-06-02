#include "Object3d.hlsli"

struct Material
{
    float4 color;
};

ConstantBuffer<Material> gMaterial0 : register(b0);
ConstantBuffer<Material> gMaterial1 : register(b1);
ConstantBuffer<Material> gMaterial2 : register(b2);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4> gTexture0 : register(t3);
Texture2D<float4> gTexture1 : register(t4);

SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float4 textureColor0 = gTexture0.Sample(gSampler, input.texcood);
    float4 textureColor1 = gTexture1.Sample(gSampler, input.texcood);

    float4 color0 = gMaterial0.color * textureColor0;
    float4 color1 = gMaterial1.color * textureColor1;
    float4 color2 = gMaterial2.color * textureColor0;

    output.color = color0 + color1 + color2;
    
    return output;
}