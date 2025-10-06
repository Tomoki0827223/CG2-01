#pragma once

#include "Vector3.h"

struct Matrix4x4 final {
    float m[4][4];

    // --- 【実装】静的関数を追加 ---

    // 単位行列を生成
    static Matrix4x4 Identity() {
        Matrix4x4 result = {};
        result.m[0][0] = 1.0f;
        result.m[1][1] = 1.0f;
        result.m[2][2] = 1.0f;
        result.m[3][3] = 1.0f;
        return result;
    }

    // 平行移動行列を生成
    static Matrix4x4 Translate(const Vector3& translate) {
        Matrix4x4 result = Identity();
        result.m[3][0] = translate.x;
        result.m[3][1] = translate.y;
        result.m[3][2] = translate.z;
        return result;
    }

    // スケール行列を生成
    static Matrix4x4 Scale(const Vector3& scale) {
        Matrix4x4 result = Identity();
        result.m[0][0] = scale.x;
        result.m[1][1] = scale.y;
        result.m[2][2] = scale.z;
        return result;
    }

    // --- 【実装】Vector3を取り出す関数を追加 ---
    // 平行移動成分をVector3として返す
    Vector3 GetTranslate() const {
        return { m[3][0], m[3][1], m[3][2] };
    }

    // --- 【実装】演算子オーバーロードを追加 ---
    // 行列同士の乗算
    Matrix4x4 operator*(const Matrix4x4& other) const {
        Matrix4x4 result = {};
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    sum += m[i][k] * other.m[k][j];
                }
                result.m[i][j] = sum;
            }
        }
        return result;
    }
};