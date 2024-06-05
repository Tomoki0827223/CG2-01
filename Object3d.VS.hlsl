#include "Object3d.hlsli"

struct TransformationMatrix
{
    float32_t4x4 WVP;
    
};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t4 texcond : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput outout;
    
    outout.position = mul(input.position, gTransformationMatrix.WVP);
    
    return outout;
}
