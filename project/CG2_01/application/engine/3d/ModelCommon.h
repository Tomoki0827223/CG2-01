#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "DirectXCommon.h" // DirectXCommonの定義が必要

// 前方宣言
class DirectXCommon;

class ModelCommon
{
public:

    // 初期化
    void Initialize(DirectXCommon* dxCommon);

    // Getter
    DirectXCommon* GetDXCommon() const { return dxCommon_; } //

private:

    // DirectX基盤のポインタ
    DirectXCommon* dxCommon_ = nullptr;
};