#pragma once

#include <list>
#include <memory>
#include "Particle.h"
#include "Vector3.h"

// 前方宣言
class Model;
class Object3dCommon;
class Camera;

// パーティクル群の生成と管理を行うエミッタークラス
class ParticleEmitter {
private:
    // パーティクル単体の描画に必要なリソース
    Object3dCommon* object3dCommon_ = nullptr; // Object3dの共通設定
    Model* particleModel_ = nullptr;           // パーティクルに使用するモデル
    Camera* camera_ = nullptr;                 // カメラ（Object3dに設定が必要）

    // パーティクルリスト
    std::list<std::unique_ptr<Particle>> particles_;

    // 生成上限数
    const size_t maxParticles_ = 1000;

public:
    // 初期化
    void Initialize(Object3dCommon* object3dCommon, Model* model, Camera* camera) {
        object3dCommon_ = object3dCommon;
        particleModel_ = model;
        camera_ = camera;
    }

    // パーティクル生成（エミット）
    void Emit(const Vector3& emitterPos, int count, float speedMin, float speedMax, float lifeTimeMin, float lifeTimeMax);

    // 全パーティクル更新
    void Update();

    // 全パーティクル描画
    void Draw();

private:
    // Particleに必要なObject3dインスタンスを生成するヘルパー関数
    Object3d* CreateObject3dInstance();
};