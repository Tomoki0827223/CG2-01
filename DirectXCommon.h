#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "Logger.h"
#include "StringUitilty.h"
#include "WinApp.h"

class DirectXCommon
{
public:
    void Initialize();
    void DXcommom();
    void CommandQueue();
    void SwapChain();
    void DepthBuffer();
    void InitializeDescriptorHeaps();
    void InitializeRTV();


    void InitializeDSVHeap();

    // SRVとGPUのデスクリプタハンドル取得関数
    D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

    // デスクリプタヒープを作成
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);

private:
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

    WinApp* winApp = nullptr;

    // RTV、SRV、D3Dのデスクリプタサイズ
    UINT rtvDescriptorSize = 0;
    UINT srvDescriptorSize = 0;
};
