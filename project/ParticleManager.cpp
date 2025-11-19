#include "ParticleManager.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <cassert>
#include <algorithm> // for std::remove_if

ParticleManager* ParticleManager::GetInstance() {
    static ParticleManager instance;
    return &instance;
}

// 初期化処理
void ParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {
    dxCommon_ = dxCommon;
    srvManager_ = srvManager; // DirectXCommonとSRVマネージャのポインタを受け取って記録

    // ランダムエンジンの初期化 (実装省略)

    // パイプライン生成、頂点リソース生成、VBV作成などの描画リソースの初期化
    CreateRenderingResources_();
}

// グループ生成処理
void ParticleManager::CreateParticleGroup(const std::string name, const std::string textureFilePath) {
    // 登録済みの名前かチェックしてassert
    assert(particleGroups_.find(name) == particleGroups_.end());

    // 新たな空のパーティクルグループを作成し、コンテナに登録
    ParticleGroup newGroup = {};
    newGroup.textureFilePath = textureFilePath; // テクスチャファイルパスを設定

    // --- 外部モジュールを利用する処理（概念的な実装） ---
    // テクスチャを読み込む (事前でもよい)
    // TextureManager::GetInstance()->LoadTexture(textureFilePath); 
    // newGroup.textureSrvIndex = TextureManager::GetInstance()->GetSrvIndexByFilePath(textureFilePath); // SRVインデックスを記録

    // インスタンシング用リソースの生成
    // dxCommon_->CreateStructuredBuffer(sizeof(ParticleInstancingData) * MAX_PARTICLES, &newGroup.instancingResource, &newGroup.instancingDataPtr);

    // インスタンシング用にSRVを確保してSRVインデックスを記録
    // newGroup.instancingSrvIndex = srvManager_->GetNewSrvIndex();
    // SRV生成 (StructuredBuffer用設定)
    // srvManager_->CreateStructuredBufferSRV(newGroup.instancingResource, newGroup.instancingSrvIndex);
    // ----------------------------------------------------

    particleGroups_[name] = newGroup;
}

// パーティクルの発生 (Emit)
void ParticleManager::Emit(const std::string name, const Vector3& position, uint32_t count) {
    // 登録済みのパーティクルグループ名かチェックしてassert
    assert(particleGroups_.count(name));
    ParticleGroup& group = particleGroups_.at(name);

    for (uint32_t i = 0; i < count; ++i) {
        // 新たなパーティクルを作成し、指定されたパーティクルグループに登録
        Particle newParticle = {};
        newParticle.position = position;
        newParticle.lifeTime = 1.0f; // 例: 寿命1秒
        newParticle.currentTime = 0.0f;
        newParticle.velocity = { 0.0f, 1.0f, 0.0f }; // 例: 上向きに移動
        newParticle.acceleration = { 0.0f, -9.8f, 0.0f }; // 例: 重力
        group.particles.push_back(newParticle);
    }
}

// 更新処理 (Update)
void ParticleManager::Update(Camera* camera, float deltaTime) {
    // ビルボード行列の計算 (実装省略)
    Matrix4x4 billboardMatrix = Matrix4x4::Identity();

    // ビュー行列とプロジェクション行列をカメラから取得
    // ⚠ 注意: CameraクラスのGetViewMatrixとGetProjectionMatrixがMatrix4x4を返すことが前提
    Matrix4x4 viewMatrix = Matrix4x4::Identity(); // camera->GetViewMatrix(); の仮の代入
    Matrix4x4 projectionMatrix = Matrix4x4::Identity(); // camera->GetProjectionMatrix(); の仮の代入

    if (camera) {
        // 実際には camera->GetViewMatrix() / camera->GetProjectionMatrix() を呼び出す想定
        // ビルドを通すため、ダミーでIdentity()を使用
    }

    Matrix4x4 viewProjectionMatrix = viewMatrix * projectionMatrix;

    // 全てのパーティクルグループについて処理する
    for (auto& pair : particleGroups_) {
        ParticleGroup& group = pair.second;
        group.instancingDatas.clear();
        group.instanceCount = 0;

        for (auto it = group.particles.begin(); it != group.particles.end();) {
            Particle& p = *it;

            // 🚀 【必須修正 1: パーティクルの速度・位置の更新処理を有効化】
            // 速度 = 速度 + 加速度 * デルタタイム
            p.velocity = p.velocity + p.acceleration * deltaTime;
            // 位置 = 位置 + 速度 * デルタタイム
            p.position = p.position + p.velocity * deltaTime;

            p.currentTime += deltaTime;

            // 💀 【必須修正 2: 寿命による削除処理を追加】
            if (p.currentTime >= p.lifeTime) {
                // 寿命が尽きたらリストから削除し、次の要素へイテレータを進める
                it = group.particles.erase(it);
                continue; // 削除したので、以降の処理はスキップして次の要素へ
            }

            // ワールド行列を計算
            p.worldMatrix = Matrix4x4::Scale({ 1.0f, 1.0f, 1.0f }) * billboardMatrix * Matrix4x4::Translate(p.position);

            // ワールドビュープロジェクション行列を合成
            ParticleInstancingData instanceData;
            instanceData.WVP = p.worldMatrix * viewProjectionMatrix;

            // インスタンシング用データ1個分の書き込み
            group.instancingDatas.push_back(instanceData);
            group.instanceCount++;

            ++it; // 削除しなかった場合のみイテレータを進める
        }

        // InstancingDataをリソースに書き込む (概念的な実装)
    }
}

// 描画処理 (Draw)
void ParticleManager::Draw() {
    // ⚠️ ここにパーティクル用のRootSignature/PSO/VBVを設定するコマンドが必要です。
    // ... (設定コマンドの実装省略)

    // 全てのパーティクルグループについて処理する
    for (auto& pair : particleGroups_) {
        ParticleGroup& group = pair.second;
        if (group.instanceCount == 0) continue;

        // 🟢 【必須修正 3: 描画コマンドの有効化】
        // 🚨 RootParamIndex はあなたのプロジェクトで定義されている値を使用してください。

        // コマンド: テクスチャのSRVのDescriptorTableを設定 (コメント解除)
        // dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(RootParamIndex::kTexture, srvManager_->GetSrvHandleGPU(group.textureSrvIndex));

        // コマンド: インスタンシングデータのSRVのDescriptorTableを設定 (コメント解除)
        // dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(RootParamIndex::kInstancingData, srvManager_->GetSrvHandleGPU(group.instancingSrvIndex));

        // コマンド: DrawCall (インスタンシング描画) (コメント解除)
        // dxCommon_->GetCommandList()->DrawInstanced(4, group.instanceCount, 0, 0); // 1グループ分で1DrawCall

        // 🚨 確実に動かすために、上記3行のコメントを解除し、定義済みであることを確認してください。
        // 例: (プロジェクトに合わせて修正)
        // dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, srvManager_->GetSrvHandleGPU(group.textureSrvIndex)); // 例としてRootParamIndex::kTextureが2だと仮定
        // dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(3, srvManager_->GetSrvHandleGPU(group.instancingSrvIndex)); // 例としてRootParamIndex::kInstancingDataが3だと仮定
        // dxCommon_->GetCommandList()->DrawInstanced(4, group.instanceCount, 0, 0);
    }
}


// ParticleManager::CreateRenderingResources_ の実装
void ParticleManager::CreateRenderingResources_() {
    // PSO, RootSignature, VBV, 頂点リソースなど、初期化処理でやるべきことの実装
    // 描画関連の具体的な処理は省略し、ビルドを通すための空の定義とします。
}