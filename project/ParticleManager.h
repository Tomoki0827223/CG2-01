#pragma once

#include <string>
#include <unordered_map>
#include <wrl.h> // ComPtrを使うために必要
#include <d3d12.h> // D3D12_VERTEX_BUFFER_VIEWを使うために必要
#include "ParticleGroup.h"
#include "Vector3.h"
#include "Camera.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "TextureManager.h" // ★修正: TextureManagerのクラス定義を取り込む
#include "Vector4.h" // 頂点データ用に必要
#include "Vector2.h" // 頂点データ用に必要
#include <cassert>
#include <algorithm>

// 前方宣言 (DirectX関連クラス)
// TextureManagerの完全な定義をインクルードしたので、ここでは前方宣言を削除します。（※元々なかった場合もありますが念のため確認）

// パーティクルシステム全体を管理するマネージャ (シングルトン)
class ParticleManager {
private:
    ParticleManager() = default;
    ~ParticleManager() = default;
    ParticleManager(const ParticleManager&) = delete;
    ParticleManager& operator=(const ParticleManager&) = delete;

public:
    static ParticleManager* GetInstance();

    void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager); // 初期化処理

    // --- パーティクルグループ関連 ---
    void CreateParticleGroup(const std::string name, const std::string textureFilePath); // パーティクルグループの生成と登録

    // --- メイン処理 ---
    void Emit(const std::string name, const Vector3& position, uint32_t count); // パーティクルの発生
    void Update(Camera* camera, float deltaTime);                               // 更新処理
    void Draw();                                                                // 描画処理

private:
    // DirectXリソース関連
    void CreateRenderingResources_();

    // メンバ変数
    DirectXCommon* dxCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;

    // パーティクルグループコンテナ
    std::unordered_map<std::string, ParticleGroup> particleGroups_;

    static const uint32_t MAX_PARTICLES = 1024;

    // --- パーティクル描画に必要なリソースをメンバとして追加 ---
    Microsoft::WRL::ComPtr<ID3D12RootSignature> particleRootSignature; // ルートシグネチャ
    Microsoft::WRL::ComPtr<ID3D12PipelineState> particlePipelineState; // PSO
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;           // 頂点リソース (クアッド)
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};                     // VBV

    // 頂点データ構造体 (インナークラスとして定義済み)
    struct ParticleVertexData {
        Vector4 position;
        Vector2 texcoord;
    };
};
