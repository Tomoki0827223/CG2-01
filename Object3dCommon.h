#pragma once

#include <wrl.h>
#include <d3d12.h>

class DirectXCommon; // 前方宣言

class Object3dCommon
{
public:
    // 初期化
    void Initialize(DirectXCommon* dxCommon);

private:
    // ルートシグネチャの作成
    void CreateRootSignature();
    // グラフィックスパイプラインの生成
    void CreateGraphicsPipeline();

    void CommonDrawSetting();

    DirectXCommon* dxCommon_ = nullptr;

    // ルートシグネチャ、パイプラインステートのメンバ変数（型は適宜修正）
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
};