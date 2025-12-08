// resources/shaders/Particle.VS.hlsl

// 頂点シェーダーの入力構造体 (Particle.hの VertexData に対応)
struct VSInput
{
    float4 pos : POSITION; // XYZ:位置, W:スケール (scale)
    float4 color : COLOR; // RGBA:色
};

// 定数バッファ構造体 (Particle.hの ConstBufferData に対応)
cbuffer VSConstBuffer : register(b0)
{
    float4x4 viewProjectionMatrix; // ビュープロジェクション行列
    float4x4 inverseViewMatrix; // ビルボードのためのビュー行列の逆行列 (V^-1)
};

// ピクセルシェーダーへの出力構造体
struct VSOutput
{
    float4 svpos : SV_POSITION; // スクリーン座標
    float4 color : COLOR; // パーティクルの色
    float2 uv : TEXCOORD; // テクスチャ座標
};

// 四角形を生成するためのオフセット（テクスチャUV座標を兼ねる）
static const float2 offsets[4] =
{
    { -0.5f, -0.5f },
    { -0.5f, 0.5f },
    { 0.5f, -0.5f },
    { 0.5f, 0.5f },
};

// 四角形のインデックス (トライアングルストリップ)
static const uint indices[4] =
{
    0, 1, 2, 3
};

// パーティクルごとの情報をグローバルに保持
// 1インスタンスにつき1回実行されるため、このデータはインスタンス全体で共有
// float4 pos = input.pos; -> pos.w に scale が入っている
float4 particlePos = float4(0.0f, 0.0f, 0.0f, 1.0f);
float particleScale = 1.0f;
float4 particleColor = float4(1.0f, 1.0f, 1.0f, 1.0f);

// 頂点シェーダーのエントリーポイント
VSOutput main(VSInput input, uint vertID : SV_VertexID)
{
    VSOutput output;

    // 入力からパーティクル情報を取り出す
    particlePos.xyz = input.pos.xyz;
    particleScale = input.pos.w; // w成分にスケールが入っていると仮定
    particleColor = input.color;

    // どの四角形の頂点を計算するか決定 (0-3)
    uint quadIndex = vertID % 4;

    // ビルボード処理: カメラに正対する回転行列の計算
    // inverseViewMatrix の左上 3x3 行列は、カメラの回転行列 (R_cam) に対応
    // R_cam * [1, 0, 0]^T はカメラの右ベクトル
    // R_cam * [0, 1, 0]^T はカメラの上ベクトル
    float3 right = inverseViewMatrix[0].xyz;
    float3 up = inverseViewMatrix[1].xyz;

    // パーティクルのローカル頂点を計算 (オフセットとスケールを適用)
    float3 localPosition =
        right * offsets[quadIndex].x * particleScale +
        up * offsets[quadIndex].y * particleScale;

    // ワールド座標を計算: パーティクル位置 + ローカル位置
    float4 worldPos = float4(particlePos.xyz + localPosition, 1.0f);

    // プロジェクション
    output.svpos = mul(worldPos, viewProjectionMatrix);
    
    // ピクセルシェーダーに色とUVを渡す
    output.color = particleColor;
    output.uv = offsets[quadIndex] + float2(0.5f, 0.5f); // (-0.5,-0.5) to (0.5,0.5) -> (0,0) to (1,1)
    
    return output;
}