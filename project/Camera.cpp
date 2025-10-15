#include "Camera.h"
#include "affine.h" 
#include "WinApp.h" // WinApp::kClientWidth, kClientHeightにアクセスするため

// デフォルトコンストラクタ (スライド「デフォルトコンストラクタ」)
Camera::Camera() :
    aspectRatio(float(WinApp::kClientWidth) / float(WinApp::kClientHeight))
{
    // 行列の初期計算はコンストラクタ内で実行しない (Updateで計算する)
    // ただし、初期値として単位行列や初期設定行列を設定しておく方が安全
    worldMatrix = MakeIdentity4x4();
    viewMatrix = MakeIdentity4x4();
    projectionMatrix = MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);
    viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
}

// 毎フレーム行列を更新する (スライド「Update」「プロジェクション行列更新」「合成行列」)
void Camera::Update() {
    // 1. View行列関連の更新
    // worldMatrix = transformからアフィン変換行列を計算;
    worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
    // viewMatrix = worldMatrixの逆行列;
    viewMatrix = Inverse(worldMatrix);

    // 2. Projection行列関連の更新
    // projectionMatrix = 透視投影行列の生成(FoV, アスペクト比, ...);
    projectionMatrix = MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);

    // 3. 合成行列の更新
    // viewProjectionMatrix = Multiply(ビュー行列, プロジェクション行列);
    viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
}

// カメラの右ベクトルを取得 (ワールド空間)
Vector3 Camera::GetWorldRightVector() const {
    // View行列の回転成分（転置）の列ベクトルを取得
    return { viewMatrix.m[0][0], viewMatrix.m[1][0], viewMatrix.m[2][0] };
}

// カメラの上ベクトルを取得 (ワールド空間)
Vector3 Camera::GetWorldUpVector() const {
    // View行列の回転成分（転置）の列ベクトルを取得
    return { viewMatrix.m[0][1], viewMatrix.m[1][1], viewMatrix.m[2][1] };
}