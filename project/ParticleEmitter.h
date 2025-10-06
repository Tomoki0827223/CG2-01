#pragma once

#include <string>
#include "Vector3.h"
#include "Matrix4x4.h"

// パーティクル発生器
class ParticleEmitter {
public:
    // コンストラクタ: ほとんどのメンバ変数を引数として受け取る
    ParticleEmitter(const std::string& groupName, const Vector3& position, float emitFrequency, uint32_t countPerEmit);

    void Update(float deltaTime); // 更新処理（時刻を進め、頻度に基づきEmitを呼び出す）
    void Emit();                  // ParticleManager::Emit()を呼び出すだけの関数

private:
    // エミッタの設定値
    std::string groupName_;     // 発生させるパーティクルグループ名
    Matrix4x4 transform_;       // エミッタの位置・姿勢 (transform.translateを使用)
    float emitFrequency_;       // 発生頻度 (1秒あたりの回数)
    uint32_t countPerEmit_;     // 1回の発生で放出するパーティクルの数

    // 発生タイミング制御用
    float timeToNextEmit_;      // 次の発生までの残り時間
};