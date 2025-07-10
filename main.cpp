#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include "DirectXCommon.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "D3DResourceLeakChecker.h"
#include "Input.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include <fstream>
#include <sstream>
#include <cassert>
#include <vector>
#include <string>
#include "affine.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"



struct TransformVector3
{
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;

};

struct DirectionaLight
{
	Vector4 color;
	Vector3 direction;
	float intensity;
};

struct Transform1 {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

Transform1 uvTransformSprite{
	{1.0f, 1.0f, 1.0f},
	{0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f},
};

#pragma endregion

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	//D3DResourceLeakChecker LeakCheak;

	WinApp* winApp_ = nullptr;
	winApp_ = new WinApp();
	winApp_->Initialize();

#pragma region Windowの生成

	//GE3
	Input* input = nullptr;
	input = new Input();
	input->Initialize(winApp_);

	DirectXCommon* dxCommon = nullptr;
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp_);

	SpriteCommon* spriteCommon = nullptr;
	spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxCommon);

	// Spriteの生成
	Sprite* sprite = new Sprite();
	sprite->Initialize(spriteCommon);

	//ウインドウを表示する
	ShowWindow(winApp_->GetHwnd(), SW_SHOW);

#pragma endregion

	bool useMonsterBall = false;

	TransformVector3 transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	//Resourcef
	const uint32_t kSubdivision = 36;
		
	//頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexReComPtr = dxCommon->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	ID3D12Resource* vertexResource = vertexReComPtr.Get();

	// モンスターボール
	DirectX::ScratchImage mipimage2 = dxCommon->LoadTexture("resources/monsterBall.png");
	const DirectX::TexMetadata& metadata2 = mipimage2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata2);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResources2 = dxCommon->UploadTextureData(textureResource2, mipimage2);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = dxCommon->GetCPUDescriptorHandle(dxCommon->GetSRVDescriptorHeap(), dxCommon->GetDescriptorSizeSRV(), 2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = dxCommon->GetGPUDescriptorHandle(dxCommon->GetSRVDescriptorHeap(), dxCommon->GetDescriptorSizeSRV(), 2);
	dxCommon->GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);

	// uvChecker
	DirectX::ScratchImage mipimageUv = dxCommon->LoadTexture("resources/uvChecker.png");
	const DirectX::TexMetadata& metadataUv = mipimageUv.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResourceUv = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadataUv);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResourcesUv = dxCommon->UploadTextureData(textureResourceUv, mipimageUv);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescUv{};
	srvDescUv.Format = metadataUv.format;
	srvDescUv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescUv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDescUv.Texture2D.MipLevels = UINT(metadataUv.mipLevels);
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPUUv = dxCommon->GetCPUDescriptorHandle(dxCommon->GetSRVDescriptorHeap(), dxCommon->GetDescriptorSizeSRV(), 3);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPUUv = dxCommon->GetGPUDescriptorHandle(dxCommon->GetSRVDescriptorHeap(), dxCommon->GetDescriptorSizeSRV(), 3);
	dxCommon->GetDevice()->CreateShaderResourceView(textureResourceUv.Get(), &srvDescUv, textureSrvHandleCPUUv);


	dxCommon->GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);



	//DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
	DirectX::ScratchImage mipImages2 = dxCommon->LoadTexture(modelData.material.textureFilePath);
	const DirectX::TexMetadata& metadata = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResources = dxCommon->UploadTextureData(textureResource, mipImages2);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxCommon->GetSRVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxCommon->GetSRVDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	//戦闘はImGuiが使っているのでその次を使う
	textureSrvHandleCPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//SRVの生成
	dxCommon->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);

	MSG msg{};


	dxCommon->InitializeImGui();

    while (true)
    {
        if (winApp_->ProsessMeassage())
        {
            break;
        }
        else
        {
            // GE3
            input->Update();

            // ゲーム処理

            // 描画前処理
            dxCommon->PreDraw();

			// ImGuiのフレーム開始
			ImGui_ImplWin32_NewFrame();
			ImGui_ImplDX12_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("Ball Controls");
			ImGui::Checkbox("useMonsterball", &useMonsterBall);

			// 切り替え
			ImTextureID imguiTexture = reinterpret_cast<ImTextureID>(
				useMonsterBall ? textureSrvHandleGPU2.ptr : textureSrvHandleGPUUv.ptr
				);
			ImGui::Image(imguiTexture, ImVec2(128, 128));
			ImGui::End();

			// ImGui描画
			ImGui::Render();
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());
			
			spriteCommon->CommandListCreate();

			// 修正後（正しい）
			dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(
				2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPUUv
			);

			dxCommon->InitializeViewportAndScissorRect();
			dxCommon->InitializeScissorRect();

			sprite->Update();
			sprite->Draw();

			dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);

            // 描画後処理
            dxCommon->PostDraw();
        }
    }


	//Windows終了
	winApp_->Finalize();

	delete input;
	delete winApp_;
	delete dxCommon;
	delete spriteCommon;

	return 0;
}