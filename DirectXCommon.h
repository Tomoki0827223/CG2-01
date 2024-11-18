#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <format>
#include <dxcapi.h>
#include <vector>
#include <numbers>
#include <fstream>
#include <sstream>
#include "WinApp.h"

#include "Logger.h"
#include "StringUitilty.h"
#include "externals/DirectXTex/d3dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/imgui/imgui_impl_dx12.h"


class DirectXCommon
{
public:

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

    IDxcBlob* CompileShader(const std::wstring& filePath, const wchar_t* profile);

    // ImGuiの初期化
    void InitializeImGui();


    void Initialize(WinApp* winApp);

    // SRVとGPUのデスクリプタハンドル取得関数
    D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

    // デスクリプタヒープを作成
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);

    // DirectX 12 で使うリソースやハンドル
    Microsoft::WRL::ComPtr<ID3D12Device> device;
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;

    // コマンドキュー、コマンドアロケータ、コマンドリスト
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;


    Microsoft::WRL::ComPtr<IDXGISwapChain1> tempSwapChain = nullptr;
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;

    Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob;
    Microsoft::WRL::ComPtr<IDxcBlobUtf16> shaderOutputName;

    Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;

    WinApp* winApp_ = nullptr;

    D3D12_VIEWPORT viewport_;
    D3D12_RECT scissorRect_;

    // RTV、SRV、D3Dのデスクリプタサイズ
    UINT rtvDescriptorSize = 0;
    UINT srvDescriptorSize = 0;

    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils;
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler;
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;

private:
};
