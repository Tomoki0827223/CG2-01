#pragma once

#include "Vector3.h"
#include "Matrix4x4.h"

// 1粒のパーティクルデータ
struct Particle {
    Vector3 position;   // 座標
    Vector3 velocity;   // 速度
    Vector3 acceleration; // 加速度 (場の影響)
    float lifeTime;     // 寿命 (初期値)
    float currentTime;  // 経過時間
    Matrix4x4 worldMatrix; // ワールド行列
};

// インスタンシング描画用データ (Shaderに送る)
struct ParticleInstancingData {
    Matrix4x4 WVP;      // ワールドビュープロジェクション行列
    // 他にも色、サイズなどを追加する
};