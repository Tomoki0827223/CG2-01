// project/Particle.h

#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <vector>
#include <memory> // std::unique_ptr
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"

// 既存のコードに追加・修正

class Particle {
public:
    // 最大パーティクル数 (仮)
    static const int32_t kMaxParticles = 1024;

private:
    // スクリーンショット1: 頂点シェーダー入力構造体 (VSInput) に対応
    // この構造体のインスタンスが、kMaxParticles分CPU側に保持され、更新時にGPUに転送されます。
    struct VertexData {
        Vector3 pos;   // POSITION
        float scale;   // SIZE (W軸を利用、HLSLのfloat4のWはsizeとして使用)
        Vector4 color; // COLOR
    };

    // スクリーンショット1: 定数バッファ構造体 (VSConstBuffer) に対応
    struct ConstBufferData {
        Matrix4x4 viewProjectionMatrix; // ビュープロジェクション行列
        Matrix4x4 inverseViewMatrix;    // ビルボードのためのビュー行列の逆行列 (V^-1)
    };

    // パーティクルごとの情報（CPU側で更新）
    struct ParticleData {
        Vector3 position;
        Vector3 velocity;
        int32_t lifeTime;      // 寿命(フレーム)
        int32_t currentTime;   // 現在時間(フレーム)
        Vector4 startColor;
        Vector4 endColor;
        float startScale;
        float endScale;
        bool isActive;         // 有効/無効 フラグ
    };

private:
    // DirectX 関連リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> vbResource;    // 頂点バッファ
    D3D12_VERTEX_BUFFER_VIEW vbView{};                     // 頂点バッファビュー

    Microsoft::WRL::ComPtr<ID3D12Resource> ibResource;    // インデックスバッファ
    D3D12_INDEX_BUFFER_VIEW ibView{};                      // インデックスバッファビュー

    Microsoft::WRL::ComPtr<ID3D12Resource> cbResource;    // 定数バッファ
    ConstBufferData* constMap = nullptr;                   // 定数バッファのマッピング済みポインタ

    // パイプライン
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;

    // シェーダーリソースビュー (SRV)
    uint32_t srvHandle; // テクスチャ用SRVのハンドル (SrvManagerを使用している場合)
    uint32_t textureHandle; // テクスチャ読み込み後のハンドル (TextureManagerを使用している場合)

    // CPU側データ
    VertexData* vertexData = nullptr;                     // 頂点バッファのマッピング済みポインタ
    std::vector<ParticleData> particles;                  // パーティクル実体リスト

public:
    Particle();
    ~Particle();

    void Initialize(uint32_t textureHandle);
    void Update();
    void Draw(ID3D12GraphicsCommandList* commandList, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& inverseViewMatrix);

    // パーティクル生成関数
    void Emit(const Vector3& position, const Vector3& velocity, int32_t lifeTime, float startScale, float endScale, const Vector4& startColor, const Vector4& endColor);

private:
    void CreateResources();
    void CreatePipeline();
    void InitializeVertexData();
};