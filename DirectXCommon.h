#pragma once
#include "WinApp.h"

class DirectXCommon
{
public:

	void Initialize(WinApp* winApp);

    // デバイスの生成
    void CreateDevice();
    // コマンド関連の初期化
    void InitializeCommandObjects();
    // スワップチェーンの生成
    void CreateSwapChain();
    // 深度バッファの生成
    void CreateDepthBuffer();
    // 各種デスクリプタヒープの生成
    void CreateDescriptorHeaps();
    // レンダーターゲットビューの初期化
    void InitializeRenderTargetView();
    // 深度ステンシルビューの初期化
    void InitializeDepthStencilView();
    // フェンスの初期化
    void InitializeFence();
    // ビューポート矩形の初期化
    void InitializeViewportAndScissorRect();
    // DXCコンパイラの生成
    void CreateDXCCompiler();
    // ImGuiの初期化
    void InitializeImGui();

};

