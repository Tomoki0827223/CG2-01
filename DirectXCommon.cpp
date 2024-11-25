#include "DirectXCommon.h"
#include <dxcapi.h>
#include <cassert>
#include <combaseapi.h>
#include <d3d12.h>

void DirectXCommon::Initialize(WinApp* winApp) {
    // NULL検出
    assert(winApp);
    // メンバ変数に記録
    this->winApp_ = winApp;

    // デバイスの生成
    CreateDevice();
    // コマンド関連の初期化
    InitializeCommandObjects();
    // スワップチェーンの生成
    CreateSwapChain();
    // 深度バッファの生成
    CreateDepthBuffer();
    // 各種デスクリプタヒープの生成
    CreateDescriptorHeaps();
    // レンダーターゲットビューの初期化
    InitializeRenderTargetView();
    // 深度ステンシルビューの初期化
    InitializeDepthStencilView();
    // フェンスの初期化
    InitializeFence();
    // ビューポート矩形の初期化
    InitializeViewportAndScissorRect();
	//シザリング矩形の初期化
	InitializeScissorRect();
    // DXCコンパイラの生成
    CreateDXCCompiler();
    // ImGuiの初期化
	InitializeImGui();
}

void DirectXCommon::CreateDevice()
{
    
}

void DirectXCommon::InitializeCommandObjects()
{
}

void DirectXCommon::CreateSwapChain()
{
}

void DirectXCommon::CreateDepthBuffer()
{
}

void DirectXCommon::CreateDescriptorHeaps()
{
}

void DirectXCommon::InitializeRenderTargetView()
{
}

void DirectXCommon::InitializeDepthStencilView()
{
}

void DirectXCommon::InitializeFence()
{
}

void DirectXCommon::InitializeViewportAndScissorRect()
{
}

void DirectXCommon::InitializeScissorRect()
{

}

void DirectXCommon::CreateDXCCompiler()
{
}

void DirectXCommon::InitializeImGui()
{
}
