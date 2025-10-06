#pragma once

struct Vector3 {
    float x;
    float y;
    float z;

    // コンストラクタ（初期化リスト対応用）
    Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}

    // --- 【実装】Vector3とfloatの乗算演算子 ---
    Vector3 operator*(float s) const {
        return { x * s, y * s, z * s };
    }

    // --- 【実装】Vector3とVector3の加算演算子 ---
    Vector3 operator+(const Vector3& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }
};