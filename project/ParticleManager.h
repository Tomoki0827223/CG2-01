#pragma once

#include <vector>
#include <wrl.h> 
#include <d3d12.h>
#include "DirectXCommon.h" 
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Camera.h" // カメラ情報が必要
// ↓ TextureManager.hやSrvManager.hは.cppに移動（ヘッダーでインクルード重複を避けるため）
#include <cassert>
#include <algorithm>

// 1. パーティクル頂点データ構造体 (ビルボード用)
struct ParticleVertex {
    Vector3 pos;
    Vector2 uv;
};

// 2. パーティクルごとのデータ（インスタンスデータ）
struct ParticleData {
    Vector3 position;
    Vector3 velocity;
    Vector4 color;
    float startScale;
    float endScale;
    float lifetime;
    float currentTime;
    int isActive;
};

// 3. パーティクルマネージャー
class ParticleManager {
public:
    static const UINT kMaxParticles = 1024;

public:
    void Initialize(DirectXCommon* dxCommon, Camera* camera);
    void Update(float deltaTime);
    void Draw(ID3D12GraphicsCommandList* commandList);

    void Emit(const Vector3& position, const Vector3& velocity,
        const Vector4& color, float startScale, float endScale, float lifetime);

private:
    DirectXCommon* dxCommon_ = nullptr;
    Camera* camera_ = nullptr;

    // リソース
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;

    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vbView{};

    Microsoft::WRL::ComPtr<ID3D12Resource> instancingBuffer;
    ParticleData* instancingMap = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer; // ComPtr の P を大文字に修正

    int textureHandle = 0;

    // パーティクル配列
    ParticleData particles[kMaxParticles];

private:
    void CreatePipeline();
    void CreateResources();
    void LoadTexture();
};