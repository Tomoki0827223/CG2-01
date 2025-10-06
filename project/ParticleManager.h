#pragma once

#include <string>
#include <unordered_map>
#include "ParticleGroup.h"
#include "Vector3.h"
#include "Camera.h"

// 前方宣言 (DirectX関連クラス)
class DirectXCommon;
class SrvManager;

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
    // メンバ変数
    DirectXCommon* dxCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;

    // パーティクルグループコンテナ
    std::unordered_map<std::string, ParticleGroup> particleGroups_;

    static const uint32_t MAX_PARTICLES = 1024;

    // DirectXリソース関連
    // PSO, RootSignature, VBV, 頂点リソースなど (詳細は実装ファイルへ)
    void CreateRenderingResources_();
};