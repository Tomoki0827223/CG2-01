#include "Particle.hlsl"

// 頂点シェーダーへの入力 (ParticleManager::ParticleVertexに対応)
struct VSInput
{
    float3 pos : POSITION; // ビルボード四角形のローカル座標
    float2 uv : TEXCOORD;
};

// インスタンシングデータ入力 (ParticleManager::ParticleDataに対応)
// セマンティクスをカスタムして、D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA で設定
struct InstanceInput
{
    float3 instancePosition : INSTANCE_POSITION;
    float3 instanceVelocity : INSTANCE_VELOCITY; // 使わないが、構造体合わせのため含める
    float4 instanceColor : INSTANCE_COLOR;
    float instanceStartScale : INSTANCE_START_SCALE;
    float instanceEndScale : INSTANCE_END_SCALE;
    float instanceLifetime : INSTANCE_LIFETIME;
    float instanceCurrentTime : INSTANCE_CURRENT_TIME;
    int instanceIsActive : INSTANCE_IS_ACTIVE; // 使わないが、構造体合わせのため含める
};

VSOutput main(VSInput input, InstanceInput instance)
{
    VSOutput output;
    
    // 進行度 (0.0 から 1.0)
    float progress = instance.instanceCurrentTime / instance.instanceLifetime;
    
    // スケールの線形補間: startScale から endScale へ
    float currentScale = lerp(instance.instanceStartScale, instance.instanceEndScale, progress);
    
    // ビルボード処理: ローカル座標をカメラの右/上ベクトルでワールド座標に変換
    float3 worldPos = instance.instancePosition +
                      cameraRight * input.pos.x * currentScale +
                      cameraUp * input.pos.y * currentScale;

    // ビュープロジェクション変換
    output.svpos = mul(float4(worldPos, 1.0f), viewProjection);
    output.uv = input.uv;

    // 時間経過でアルファ値を減衰させる (例: 0.8から0.0へ)
    float alphaFade = lerp(1.0f, 0.0f, progress);
    float4 color = instance.instanceColor;
    color.a *= alphaFade;
    
    output.color = color;

    return output;
}