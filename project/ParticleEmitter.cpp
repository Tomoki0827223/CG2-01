#include "ParticleEmitter.h"
#include "ParticleManager.h"
#include <cmath>

// コンストラクタ
ParticleEmitter::ParticleEmitter(const std::string& groupName, const Vector3& position, float emitFrequency, uint32_t countPerEmit)
    : groupName_(groupName), emitFrequency_(emitFrequency), countPerEmit_(countPerEmit) {
    // ❌ transform_ = Matrix4x4::Translate(position); // 'Translate'が'Matrix4x4'のメンバーではない
    // 👇 Matrix4x4.hに静的関数Matrix4x4::Translateを追加することでエラー解消
    transform_ = Matrix4x4::Translate(position);
    timeToNextEmit_ = 1.0f / emitFrequency_;
}

// 更新処理
void ParticleEmitter::Update(float deltaTime) {
    // 時刻を進める
    timeToNextEmit_ -= deltaTime;

    // 発生頻度より大きいなら発生
    while (timeToNextEmit_ <= 0.0f) {
        // Emitを呼び出す
        Emit();

        // 余計に過ぎた時間も加味して頻度計算
        timeToNextEmit_ += 1.0f / emitFrequency_;
    }
}

// パーティクル発生処理
void ParticleEmitter::Emit() {
    ParticleManager::GetInstance()->Emit(
        groupName_,
        // ❌ transform_.GetTranslate(), // 'GetTranslate'が'Matrix4x4'のメンバーではない
        // 👇 Matrix4x4.hにGetTranslateを追加することでエラー解消
        transform_.GetTranslate(),
        // ❌ 引数エラー C2660: 関数に 2 個の引数を指定できません。
        // 👇 Emitの宣言が修正されたため、3つの引数を渡すことでエラー解消
        countPerEmit_
    );
}