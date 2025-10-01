#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"
#include <cstdint>

class Camera {
private:


    // Projection行列関連データ (スライド「プロジェクション行列関連データ」)
    Matrix4x4 projectionMatrix;
    float fovY = 0.45f;           // 水平方向視野角 (ここでは垂直方向FoVとして実装)
    float aspectRatio;          // アスペクト比
    float nearClip = 0.1f;      // ニアクリップ距離
    float farClip = 100.0f;     // ファークリップ距離

    // 合成行列 (スライド「合成行列」)
    Matrix4x4 viewProjectionMatrix;

public:
    
    // View行列関連データ (スライド「ビュー行列関連データ」)
    struct Transform {
        Vector3 scale = { 1.0f, 1.0f, 1.0f };
        Vector3 rotate = { 0.0f, 0.0f, 0.0f };
        Vector3 translate = { 0.0f, 0.0f, 0.0f };
    };
    Transform transform;
    Matrix4x4 worldMatrix;
    Matrix4x4 viewMatrix;
    
    // コンストラクタ (スライド「デフォルトコンストラクタ」)
    Camera();

    // 更新関数 (スライド「Update」)
    void Update();

    // --- getter (スライド「getter」) ---
    const Matrix4x4& GetWorldMatrix() const { return worldMatrix; }
    const Matrix4x4& GetViewMatrix() const { return viewMatrix; }
    const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix; }
    const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix; }

    // transform関連のgetter
    const Vector3& GetRotate() const { return transform.rotate; }
    const Vector3& GetTranslate() const { return transform.translate; }

    // --- setter (スライド「setter」) ---
    void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
    void SetTranslate(const Vector3& translate) { transform.translate = translate; }
    void SetFovY(float fovY) { this->fovY = fovY; }
    void SetAspectRatio(float aspectRatio) { this->aspectRatio = aspectRatio; }
    void SetNearClip(float nearClip) { this->nearClip = nearClip; }
    void SetFarClip(float farClip) { this->farClip = farClip; }

    // ImGuiでの操作のためにtransformを直接参照できるようにする
    // --- [🚨 修正点] Transform構造体の定義はprivateのままで、この関数をpublicにして参照を返す ---
    Transform& GetTransform() { return transform; }
    const Transform& GetTransform() const { return transform; } // const版も追加
    // -----------------------------------------------------------------------------------
};