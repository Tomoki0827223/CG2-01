#ifndef DIRECTX_COMMON_H
#define DIRECTX_COMMON_H

#include <dxcapi.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <string>
#include "WinApp.h" // WinApp の実装があるヘッダーをインクルード
#include <cassert>
#include <combaseapi.h>
#include <format>


class WinApp; // 前方宣言

class DirectXCommon {
public:
    void Initialize(WinApp* winApp); // WinApp を引数にとる Initialize メソッド

    std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;

private:
    void CreateDevice();
    void InitializeCommandObjects();
    void CreateSwapChain();
    void CreateDepthBuffer();
    void CreateDescriptorHeaps();
    void InitializeRenderTargetView();
    void InitializeDepthStencilView();
    void InitializeFence();
    void InitializeViewportAndScissorRect();
    void CreateDXCCompiler();
    void InitializeImGui();

    void Log(const std::string& message);

    std::string ConvertString(const std::wstring& str);
    
    // DXGI、D3D12関連メンバ
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
    Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
    
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>commandAllocator = nullptr;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;

    Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;



    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;

    // DXC関連メンバ
    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils;
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler;
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;

    HANDLE fenceEvent = nullptr;
    uint64_t fenceValue = 0;
    WinApp* winApp_ = nullptr;
};

#endif // DIRECTX_COMMON_H
