#include "ParticleEmitter.h"
#include "Object3dCommon.h" // Object3d::Initializeに必要
#include <random>
#include <ctime> // 乱数のシードに必要

// 乱数ジェネレーター
std::mt19937 randomEngine(static_cast<unsigned int>(time(nullptr)));

// ----------------------------------------------------------------------

// Particleに必要なObject3dインスタンスを生成するヘルパー関数
Object3d* ParticleEmitter::CreateObject3dInstance() {
    // Object3dを動的に生成し、共通設定とモデルを設定する
    Object3d* newObject = new Object3d();
    newObject->Initialize(object3dCommon_);

    // Model*を直接設定（Object3d.hで追加したSetModelを使用）
    newObject->SetModel(particleModel_);

    // カメラの設定（ビルボード処理などに使われる場合）
    newObject->camera = camera_;

    return newObject;
}

void ParticleEmitter::Emit(const Vector3& emitterPos, int count, float speedMin, float speedMax, float lifeTimeMin, float lifeTimeMax) {
    if (particles_.size() >= maxParticles_) {
        return;
    }

    // 乱数分布
    std::uniform_real_distribution<float> distSpeed(speedMin, speedMax);
    std::uniform_real_distribution<float> distLifeTime(lifeTimeMin, lifeTimeMax);
    // 球状に発射するための乱数 (-1.0f ～ 1.0f)
    std::uniform_real_distribution<float> distDir(-1.0f, 1.0f);

    for (int i = 0; i < count; ++i) {
        if (particles_.size() >= maxParticles_) {
            break;
        }

        // 1. 新しいParticleオブジェクトを作成
        std::unique_ptr<Particle> newParticle = std::make_unique<Particle>();

        // 2. 描画用のObject3dインスタンスを作成し、Particleに設定
        Object3d* obj = CreateObject3dInstance();
        newParticle->SetObject3d(obj);


        // 3. 初期速度をランダムに決定
        float speed = distSpeed(randomEngine);
        // 発射方向をランダムに決定し、速度を掛ける
        Vector3 velocity = {
            distDir(randomEngine) * speed,
            (distDir(randomEngine) * 0.5f + 0.5f) * speed, // Y方向は重力に逆らうように少し上向きのバイアスをかける
            distDir(randomEngine) * speed
        };

        // 4. 加速度（ここでは単純な重力のみを想定）
        Vector3 acceleration = { 0.0f, -9.8f, 0.0f };

        // 5. 初期化
        newParticle->Initialize(
            emitterPos,
            velocity,
            acceleration,
            distLifeTime(randomEngine)
        );

        // 6. リストに追加
        particles_.push_back(std::move(newParticle));
    }
}

void ParticleEmitter::Update() {
    // リストのイテレータを使用して更新
    auto it = particles_.begin();
    while (it != particles_.end()) {
        // 更新
        (*it)->Update();

        // 寿命が尽きていたら削除
        if ((*it)->IsDead()) {
            // unique_ptrにより、Particleのデストラクタが呼ばれ、その中でObject3d*がdeleteされます
            it = particles_.erase(it);
        }
        else {
            ++it;
        }
    }
}

void ParticleEmitter::Draw() {
    for (auto& particle : particles_) {
        particle->Draw();
    }
}