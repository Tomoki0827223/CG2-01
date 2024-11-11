#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

using namespace Microsoft::WRL;

#include "DirectXCommon.h"
#include <cassert>
#include <format>

void DirectXCommon::Initialize()
{
	assert(winApp);
	this->winApp = winApp;

	HRESULT hr;

}

void DirectXCommon::DXcommom()
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

void DirectXCommon::CommandQueue()
{
	HRESULT hr;

	//コマンドキュー生成
	//Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));

	//コマンドキューが生成できないので起動できない
	assert(SUCCEEDED(hr));

	//コマンドアロケータを生成する
	//Microsoft::WRL::ComPtr<ID3D12CommandAllocator>commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//コマンドアロケータを生成出来ないので起動できない
	assert(SUCCEEDED(hr));

	//コマンドリストを生成する
	//Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	//コマンドリストetc...
	assert(SUCCEEDED(hr));
}

void DirectXCommon::SwapChain()
{
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {

	//	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;

	//	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	//	descriptorHeapDesc.Type = heapType;
	//	descriptorHeapDesc.NumDescriptors = numDescriptors;
	//	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	//	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));

	//	assert(SUCCEEDED(hr));
	//	return descriptorHeap;
	//}

	//SwapChain
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = WinApp::kClientWidth;
	swapChainDesc.Height = WinApp::kClientHeight;//画面の高さ
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
	swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//描画のターゲットとして利用する
	swapChainDesc.BufferCount = 2;//ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニターに移したら中身を破壊

	HRESULT hr;

	hr = degiFactory->CreateSwapChainForHwnd(

		commandQueue.Get(),
		
		winApp->GetHwnd(),
		&swapChainDesc,
		nullptr,
		nullptr,
		reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));

	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);

	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	//swapchainからリソースを引っ張る
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2] = { nullptr };

	//うまくできなければ起動できない
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));
}

void DirectXCommon::DepthBuffer()
{
	//Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height)
	//{

	//	D3D12_RESOURCE_DESC resourceDesc{};
	//	resourceDesc.Width = width;
	//	resourceDesc.Height = height;
	//	resourceDesc.MipLevels = 1;
	//	resourceDesc.DepthOrArraySize = 1;
	//	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//	resourceDesc.SampleDesc.Count = 1;
	//	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	//	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;


	//	D3D12_HEAP_PROPERTIES heapProperties{};
	//	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	//	D3D12_CLEAR_VALUE depthClearValue{};
	//	depthClearValue.DepthStencil.Depth = 1.0f;
	//	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//	//Resourceの生成
	//	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	//	HRESULT hr = device->CreateCommittedResource(
	//		&heapProperties,
	//		D3D12_HEAP_FLAG_NONE,
	//		&resourceDesc,
	//		D3D12_RESOURCE_STATE_DEPTH_WRITE,
	//		&depthClearValue,
	//		IID_PPV_ARGS(&resource));
	//	assert(SUCCEEDED(hr));
	//	return resource;
	//}



	//ここから03_01
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	//ここから03_01

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResouce = CreateDepthStencilTextureResource(device, WinApp::kClientWidth, WinApp::kClientHeight);

	//ここから03_01
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(depthStencilResouce.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	//ここから03_01

	//DepthStencilTextureを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = CreateDepthStencilTextureResource(device, WinApp::kClientWidth, WinApp::kClientHeight);
}
