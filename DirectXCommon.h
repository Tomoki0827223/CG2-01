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

	//Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType,UINT numDescriptors, bool shaderVisible);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);


private:

	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<IDXGIFactory7> degiFactory;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>commandAllocator = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	//Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;

	WinApp* winApp = nullptr;
};

