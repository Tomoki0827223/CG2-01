#include "Object3d.hlsli"

struct TransformationMatrix
{
    float4x4 WVP;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    output.position = mul(float4(input.position.xyz, 1.0f), gTransformationMatrix.WVP);
    output.texcood = input.texcoord;
    
    return output;
}
