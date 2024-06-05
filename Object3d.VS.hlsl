#include "Object3d.hlsli"

struct TransformationMatrix
{
    float32_t4x4 WVP;
};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct VertexShaderInput
{
    float32_t4 position : POSITION;
    float32_t2 texcoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;

    // 頂点の位置をワールドビュー射影行列で変換
    output.position = mul(input.position, gTransformationMatrix.WVP);

    // テクスチャ座標をそのまま渡す
    output.texcood = input.texcoord;
    
    return output;
}
