#include "Object3d.hlsl"

struct Material
{
    float32_t4 color;
    int32_t enableLighting;
    float32_t shininess;
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

struct Camera
{
    float32_t3 WorldPosition;
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);

PixelShaderOutput main(VertexShanderOutput input)
{
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    PixelShaderOutput output;
    float32_t3 toEye = normalize(gCamera.WorldPosition - input.WorldPosition);
    float32_t3 reflectLight = reflect(gDirectionalLight.direction, normalize(input.normal));
    
    float RdotE = dot(reflectLight, toEye);
    float specularPow = pow(saturate(RdotE), gMaterial.shininess);
    //ここに書きていくうう
    float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
    float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    
    //拡散反射光
    float32_t4 diffuse = gMaterial.color.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
    //鏡面反射光
    float32_t3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float32_t3(1.0f, 1.0f, 1.0f);
    //拡散反射光+鏡面反射光
    output.color.rgb = diffuse + specular;
    //アルファは今まで通り
    output.color.a = gMaterial.color.a * textureColor.a;
    
    //if (gMaterial.enableLighting != 0)
    //{
        
    //    output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
    //}
    //else
    //{ // Lightingしない場合。前回までと同じ演算
    //    output.color = gMaterial.color * textureColor;
    //}
    //return output;
}