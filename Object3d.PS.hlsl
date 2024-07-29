#include "Object3d.hlsl"

struct Material
{
    float32_t4 color;
    int32_t enableLighting;
    float32_t3x3 uvTransform;
};

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

struct DirectionalLight
{
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

PixelShaderOutput main(VertexShanderOutput input)
{
    //float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    float3 transformdUV = mul(float32_t3(input.texcoord, 1.0f), gMaterial.uvTransform);
    float32_t textureColor = gTexture.Sample(gSampler, transformdUV.xy);
    
    PixelShaderOutput output;
    
    if (gMaterial.enableLighting != 0)
    {
        //ここに書きていくうう
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
    }
    else
    { // Lightingしない場合。前回までと同じ演算
        output.color = gMaterial.color * textureColor;
    }
    return output;
}