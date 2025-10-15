#pragma once

#include <vector>
#include <wrl.h> // ComPtr の定義が含まれている
#include <d3d12.h>
#include "DirectXCommon.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Camera.h" // カメラ情報が必要
// 以下のインクルードはクラス定義には不要なので削除し、.cppファイルに移動します。
// #include "TextureManager.h" 
// #include "SrvManager.h"
#include <cassert>
#include <algorithm>

// 1. パーティクル頂点データ構造体 (ビルボード用)
// 単位四角形 (0,0,0) を中心とした 4頂点
struct ParticleVertex {
    Vector3 pos; // ローカル座標 (x, y)
    Vector2 uv;  // UV座標
};

// 2. パーティクルごとのデータ（インスタンスデータ）
struct ParticleData {
    Vector3 position;    // ワールド座標
    Vector3 velocity;    // 速度
    Vector4 color;       // 色 (R, G, B, A)
    float startScale;    // 開始時のスケール
    float endScale;      // 終了時のスケール
    float lifetime;      // 最大生存時間
    float currentTime;   // 現在の経過時間
    int isActive;        // 有効フラグ (1:Active, 0:Inactive)
};

// 3. パーティクルマネージャー
class ParticleManager {
public:
    // 最大パーティクル数
    static const UINT kMaxParticles = 1024;

public:
    void Initialize(DirectXCommon* dxCommon, Camera* camera);
    void Update(float deltaTime);
    void Draw(ID3D12GraphicsCommandList* commandList);

    // パーティクル放出機能
    void Emit(const Vector3& position, const Vector3& velocity,
        const Vector4& color, float startScale, float endScale, float lifetime);

private:
    DirectXCommon* dxCommon_ = nullptr;
    Camera* camera_ = nullptr;

    // リソース
    // 💡 修正: Comptr -> ComPtr (Pを大文字に)
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;

    // ビルボード用頂点リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vbView{};

    // インスタンスデータ用リソース (kMaxParticles 分の ParticleData)
    Microsoft::WRL::ComPtr<ID3D12Resource> instancingBuffer;
    ParticleData* instancingMap = nullptr; // CPUからアクセスするためのポインタ

    // 定数バッファ (WVP行列やカメラのベクトルなど)
    // 💡 修正: Comptr -> ComPtr (Pを大文字に)
    Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer;

    // テクスチャハンドル (TextureManagerでロードしたテクスチャのID)
    int textureHandle = 0;

    // パーティクル配列
    ParticleData particles[kMaxParticles];

private:
    void CreatePipeline();
    void CreateResources();
    // 適切なテクスチャをロード
    void LoadTexture();
};