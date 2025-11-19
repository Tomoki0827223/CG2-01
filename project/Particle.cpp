#include "Particle.h"
#include <algorithm> // std::clamp用
#include <cmath>     // std::lerpなどの算術関数用

// 注意: Vector3の演算子（+、*など）がaffine.h/cppなどで定義されていることを前提とします。

void Particle::Initialize(const Vector3& position, const Vector3& velocity, const Vector3& accel, float lifeTime) {
    position_ = position;
    velocity_ = velocity;
    acceleration_ = accel;
    lifeTime_ = lifeTime;
    lifeTimer_ = 0.0f;

    // 描画オブジェクトの初期設定
    if (object3d_) {
        object3d_->SetScale({ startScale_, startScale_, startScale_ });
        object3d_->SetRotate({ 0, 0, 0 });
        object3d_->SetTranslate(position_);
        object3d_->Update();
    }
}

void Particle::Update() {
    if (IsDead()) {
        return;
    }

    float deltaTime = 1.0f / 60.0f; // 60FPS固定と仮定 (理想的にはフレーム間の実時間を取得すべき)

    // 1. 生存時間更新
    lifeTimer_ += deltaTime;

    // 2. 物理計算 (Euler法)
    // 速度 = 速度 + 加速度 * 時間
    velocity_.x += acceleration_.x * deltaTime;
    velocity_.y += acceleration_.y * deltaTime;
    velocity_.z += acceleration_.z * deltaTime;

    // 位置 = 位置 + 速度 * 時間
    position_.x += velocity_.x * deltaTime;
    position_.y += velocity_.y * deltaTime;
    position_.z += velocity_.z * deltaTime;

    // 3. 描画用Object3dの更新
    if (object3d_) {
        // 進行度を計算 (0.0f -> 1.0f)
        float lifeRate = std::clamp(lifeTimer_ / lifeTime_, 0.0f, 1.0f);

        // スケールを線形補間 (startScale -> endScale)
        float currentScale = startScale_ * (1.0f - lifeRate) + endScale_ * lifeRate;

        // 位置とスケールをObject3dに反映
        object3d_->SetTranslate(position_);
        object3d_->SetScale({ currentScale, currentScale, currentScale });

        // Object3d自身のUpdate（ワールド行列の更新など）を呼び出す
        object3d_->Update();
    }
}

void Particle::Draw() {
    if (!IsDead() && object3d_) {
        object3d_->Draw();
    }
}