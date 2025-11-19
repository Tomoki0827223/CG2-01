#pragma once

#include "Vector3.h"
#include "Vector4.h"
#include "Object3d.h"

// パーティクル単体のデータとロジックを管理するクラス
class Particle {

public:
    // **注意** ParticleEmitter内でObject3d*のメモリ管理を行うため、
    // ParticleEmitter内でObject3d*にアクセスできるよう、ここではObject3d*をpublicにする、
    // もしくはParticleEmitterをフレンドクラスに設定するなどの対応が必要です。
    // 便宜上、ここでは一時的にpublicとしておきます。
    Object3d* object3d_ = nullptr; // 描画処理を担うObject3dのポインタ

public:
    Particle() = default;
    ~Particle() { delete object3d_; object3d_ = nullptr; } // Particleが破棄されるときにObject3dも破棄

    // 初期化（生成時のパラメータ設定）
    void Initialize(const Vector3& position, const Vector3& velocity, const Vector3& accel, float lifeTime);

    // 更新処理
    void Update();

    // 描画処理 (Object3d::Drawを呼び出す)
    void Draw();

    // 生存フラグの確認
    bool IsDead() const { return lifeTimer_ >= lifeTime_; }

    // Object3dインスタンスを設定する
    void SetObject3d(Object3d* object3d) { object3d_ = object3d; }

private:
    // パーティクルの動きと外観に関するデータ
    Vector3 position_;     // 位置
    Vector3 velocity_;     // 速度
    Vector3 acceleration_; // 加速度

    float lifeTime_ = 0.0f;   // 最大生存時間
    float lifeTimer_ = 0.0f;  // 生存時間タイマー

    float startScale_ = 0.3f; // 初期スケール
    float endScale_ = 0.0f;   // 最終スケール
};