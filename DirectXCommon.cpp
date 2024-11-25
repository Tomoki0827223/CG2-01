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
#ifdef _DEBUG

	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();

		debugController->SetEnableSynchronizedCommandQueueValidation(TRUE);

	}

#endif

#pragma region DXGIFactryの生成
	//dxgiFactoryの生成
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	//HRESULT→Windows系のエラーコード
	//関数が成功したかどうかをSUCCEEDEDマクロで判定できる
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useadapter = nullptr;
	assert(SUCCEEDED(hr));
#pragma endregion


#pragma region 使用アダプタ(GPU)の決定

	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useadapter)) !=
		DXGI_ERROR_NOT_FOUND; ++i) {

		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useadapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));

		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			Logger::Log(StringUitilty::ConvertString(std::format(L"Use Adapater:{}\n", adapterDesc.Description)));
			break;
		}
		useadapter = nullptr;
	}

	assert(useadapter != nullptr);

#pragma endregion

#pragma region D3D12Deviceの生成

	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
	D3D_FEATURE_LEVEL featureLevels[] = {

		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0

	};
	const char* featureLevelStrrings[] = { "12.2","12.1","12.0" };

	for (size_t i = 0; i < _countof(featureLevels); i++)
	{
		hr = D3D12CreateDevice(useadapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));

		if (SUCCEEDED(hr))
		{
			Logger::Log(std::format("FeatureLevel : {}\n", featureLevelStrrings[i]));
			break;
		}
	}
	//デバイスの生成がうまくいかなかったので起動できない
	assert(device != nullptr);

	Logger::Log("Complete create D3D12Device!!!\n");//初期化完了のログを出す

#pragma endregion

#ifdef _DEBUG

	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
	{
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		//抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {

			//Windows11でのDXGIでバックプレイヤーとDX12デバッグレイヤーの互換作用バグによるエラーメッセージ
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};

		//抑制するレベル
		D3D12_MESSAGE_SEVERITY serverities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(serverities);
		filter.DenyList.pSeverityList = serverities;

		//指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);

	}

#endif // _DEBUG
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
