#include "Object3d.h"
#include "Object3dCommon.h" // Object3dCommonの関数呼び出しのためにインクルード
#include "DirectXCommon.h" // リソース作成のために必要
#include <fstream>
#include <sstream>
#include <cassert>
#include "affine.h"

void Object3d::Initialize(Object3dCommon* object3dCommon)
{
    this->object3dCommon = object3dCommon;

    // --- 【修正】モデル読み込みとリソース作成の処理はModelに移行したため削除 ---
    // modelData = LoadObjFile(...); // 削除
    // if (!modelData.material.textureFilePath.empty()) { ... } // 削除

    // Object Transformの初期設定
    transform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

    // Camera Transformの初期設定
    cameraTransform = { {1.0f, 1.0f, 1.0f}, {0.3f, 0.0f, 0.0f}, {0.0f, 4.0f, -10.0f} };

    // 2. 各種リソースの作成（Object3dに残るもののみ）
    // CreateVertexData();     // 削除
    // CreateMaterialData();   // 削除
    CreateTransformationMatrixData();
    CreateDirectionalLightData();
}


void Object3d::Update()
{
    // 修正案: 1/10の速度にする
    transform.rotate.y += 0.01f;

    // --- [🚨 追加箇所 5: World-View-Projection行列の計算 🚨] ---
    // 1. TransformからWorldMatrixを作る
    Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

    // 2. cameraTransformからCameraMatrixを作る
    Matrix4x4 cameraMatrix = MakeAffineMatrix(
        cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate
    );

    // 3. cameraMatrixからViewMatrixを作る
    Matrix4x4 viewMatrix = Inverse(cameraMatrix);

    // 4. ProjectionMatrixを作る (ここでは透視投影)
    Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(
        0.45f, (float)WinApp::kClientWidth / (float)WinApp::kClientHeight, 0.1f, 100.0f
    );
    // ※ WinApp::kClientWidth/kClientHeight にアクセスできる必要があります。

    // 5. 結果をTransformationMatrixDataに書き込む
    transformationMatrixData->WVP = Multiply(
        worldMatrix, Multiply(viewMatrix, projectionMatrix)
    );
    transformationMatrixData->World = worldMatrix;
    // ----------------------------------------------------------------------
}

void Object3d::Draw()
{
    // コマンドリスト取得
    auto commandList = object3dCommon->GetDXCommon()->GetCommandList();

    // --- Modelの描画に必要なObject3d固有の設定のみ残す ---

    // 1. VertexBufferViewを設定 (Model.Draw()に移行) // 削除
    // 2. マテリアルCBufferの場所を設定 (Model.Draw()に移行) // 削除

    // 3. 座標変換行列CBufferの場所を設定 (RootParameter 1)
    commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

    // 4. SRVのDescriptorTableの先頭を設定 (Model.Draw()に移行) // 削除

    // 5. 平行光源CBufferの場所を設定 (RootParameter 3)
    commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

    // --- 【追加】Modelが割り当てられていればModelのDrawを呼ぶ ---
    if (model) { //
        model->Draw(); //
    }
    // -------------------------------------------------------------------

    // 6. 描画! (DrawCall) (Model.Draw()に移行) // 削除
}

void Object3d::CreateTransformationMatrixData()
{
    auto dxCommon = object3dCommon->GetDXCommon();

    // 座標変換行列リソースを作る
    transformationMatrixResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));

    // データ書き込みアドレスを取得し、transformationMatrixDataに割り当てる
    transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));

    // 単位行列を書き込んでおく
    transformationMatrixData->WVP = MakeIdentity4x4();
    transformationMatrixData->World = MakeIdentity4x4();
}

void Object3d::CreateDirectionalLightData()
{
    auto dxCommon = object3dCommon->GetDXCommon();

    // 平行光源リソースを作る
    directionalLightResource = dxCommon->CreateBufferResource(sizeof(DirectionalLight));

    // データ書き込みアドレスを取得し、directionalLightDataに割り当てる
    directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));

    // デフォルト値を書き込んでおく (自分で考える箇所)
    // 例: 真上から当たる白い光
    directionalLightData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    directionalLightData->direction = { 0.0f, -1.0f, 0.0f }; // Y軸下向き（真上から）
    directionalLightData->intensity = 1.0f;
}