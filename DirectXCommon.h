#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <format>
#include <dxcapi.h>
#include <vector>
#include <numbers>
#include <fstream>
#include <sstream>
#include <array>
#include "WinApp.h"

#include "Logger.h"
#include "StringUitilty.h"
#include "externals/DirectXTex/d3dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/imgui/imgui_impl_dx12.h"

#include "externals/DirectXTex/DirectXTex.h"


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
	//シザリング矩形の初期化
	void InitializeScissorRect();
    // DXCコンパイラの生成
    void CreateDXCCompiler();

    // 描画前処理
    void PreDraw();
    // 描画後処理
    void PostDraw();

    IDxcBlob* CompileShader(const std::wstring& filePath, const wchar_t* profile);

    // ImGuiの初期化
    void InitializeImGui();

    void Initialize(WinApp* winApp);

    // SRVとGPUのデスクリプタハンドル取得関数
    static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>&descriptorHeap, uint32_t descriptorSize, uint32_t index);
    static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>&descriptorHeap, uint32_t descriptorSize, uint32_t index);

    D3D12_CPU_DESCRIPTOR_HANDLE GetsrvCPUDescriptorHandle(uint32_t index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetsrvGPUDescriptorHandle(uint32_t index);

    Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes);

    // デスクリプタヒープを作成
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

    Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages, Microsoft::WRL::ComPtr<ID3D12Device> device,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);

    Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metadata);
    //Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);

    // device のゲッター関数
    Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const { return device; }

	// コマンドキューのゲッター関数
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() const { return commandQueue; }

	// コマンドアロケータのゲッター関数
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> GetCommandAllocator() const { return commandAllocator; }

	// コマンドリストのゲッター関数
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() const { return commandList; }

	// スワップチェーンのゲッター関数
	Microsoft::WRL::ComPtr<IDXGISwapChain4> GetSwapChain() const { return swapChain; }

	// RTVディスクリプタヒープのゲッター関数
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetRTVDescriptorHeap() const { return rtvDescriptorHeap; }

    // DirectX 12 で使うリソースやハンドル
    Microsoft::WRL::ComPtr<ID3D12Device> device;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetSrvDescriptorHeap() const { return srvDescriptorHeap; }

    uint32_t descriptorSizeSRV = 0;
    uint32_t descriptorSizeRTV = 0;
    uint32_t descriptorSizeDSV = 0;

private:

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

private:

    Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob;
    Microsoft::WRL::ComPtr<IDxcBlobUtf16> shaderOutputName;

    Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	HANDLE fenceEvent = nullptr;

public:

    std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;

    //ディスクリプタの先頭を取得する
    //D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    //RTVを二つ作るのでディスクリプタを二つ用意

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

private:

    uint64_t fenceValue = 0;
    HRESULT hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

    WinApp* winApp_ = nullptr;

    D3D12_VIEWPORT viewport_;
    D3D12_RECT scissorRect_;

    // RTV、SRV、D3Dのデスクリプタサイズ
    UINT rtvDescriptorSize = 0;
    UINT srvDescriptorSize = 0;

    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils;
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler;
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;
};
