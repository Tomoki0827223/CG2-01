#include "Object3dCommon.h"
#include "DirectXCommon.h"

void Object3dCommon::Initialize(DirectXCommon* dxCommon)
{
    dxCommon_ = dxCommon;
    CreateGraphicsPipeline();
}

void Object3dCommon::CreateRootSignature()
{
    // ここにルートシグネチャ生成処理（main.cppから移植）
}

void Object3dCommon::CreateGraphicsPipeline()
{
    CreateRootSignature(); // 先にルートシグネチャを作成
    // ここにパイプライン生成処理（main.cppから移植）
}

void Object3dCommon::CommonDrawSetting()
{
    // コマンドリスト取得
    auto* commandList = dxCommon_->GetCommandList();

    // ルートシグネチャをセット
    commandList->SetGraphicsRootSignature(rootSignature_.Get());

    // パイプラインステートをセット
    commandList->SetPipelineState(pipelineState_.Get());

    // プリミティブトポロジーをセット（例: 三角形リスト）
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}