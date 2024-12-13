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

	Sprite* sprite = nullptr;
	sprite = new Sprite();
	sprite->Initialize(spriteCommon);

	//ウインドウを表示する
	ShowWindow(winApp_->GetHwnd(), SW_SHOW);

#pragma endregion

	//DirectX::ScratchImage mipimage2 = dxCommon->LoadTexture("resources/monsterBall.png");
	//const DirectX::TexMetadata& metadata2 = mipimage2.GetMetadata();
	//Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata2);
	//Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResources2 = dxCommon->UploadTextureData(textureResource2, mipimage2);



	////DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
	//DirectX::ScratchImage mipImages2 = dxCommon->LoadTexture(modelData.material.textureFilePath);
	//const DirectX::TexMetadata& metadata = mipImages2.GetMetadata();
	//Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata);
	//Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResources = dxCommon->UploadTextureData(textureResource, mipImages2);
	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	//srvDesc.Format = metadata.format;
	//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	//srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	////SRVを作成するDescriptorHeapの場所を決める
	//D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxCommon->GetSRVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	//D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxCommon->GetSRVDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	////戦闘はImGuiが使っているのでその次を使う
	//textureSrvHandleCPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//textureSrvHandleGPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	////SRVの生成
	//dxCommon->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);

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

			sprite->Update();

            // ゲーム処理

            // 描画前処理
            dxCommon->PreDraw();

			spriteCommon->CommonRenderSettings();

			//transform.rotate.y += 0.0f;
			//Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
			//Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			//Matrix4x4 viewMatrix = Inverse(cameraMatrix);
			//Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(winApp_->kClientWidth) / float(winApp_->kClientHeight), 0.1f, 100.0f);
			//Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
			//wvpDeta->world = worldMatrix;
			//wvpDeta->WVP = worldViewProjectionMatrix;


			//// Sprite用のWorldViewProjectionMatrixを作る
			//Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
			//Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
			//Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(winApp_->kClientWidth), float(winApp_->kClientHeight), 0.0f, 100.0f);
			//Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
			//transformationMatrixDataSprite->world = worldMatrixSprite;
			//transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;

			////スプライト
			//Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
			//uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
			//uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
			//materialDataSprite->uvTransform = uvTransformMatrix;

			//// ImGuiのフレーム開始
			//ImGui_ImplWin32_NewFrame();
			//ImGui_ImplDX12_NewFrame();
			//ImGui::NewFrame();

			//// ImGuiウィンドウ
			//ImGui::Begin("Ball Controls");
			//ImGui::SliderFloat3("Position", &transform.translate.x, -5.0f, 5.0f);
			//ImGui::SliderFloat3("Rotation", &transform.rotate.x, -180.0f, 180.0f);
			//ImGui::SliderFloat3("Scale", &transform.scale.x, 0.1f, 2.0f);
			////ImGui::SliderFloat("MonsterBallsc", &w, 0.1f, 2.0f);
			//ImGui::Checkbox("useMonsterball", &useMonsterBall);
			//ImGui::End();

			// ImGuiの描画
			//ImGui::Render();
			//ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

			//dxCommon->GetCommandList()->RSSetViewports(1, &viewport);
			//dxCommon->GetCommandList()->RSSetScissorRects(1, &scissorRect);

			dxCommon->InitializeViewportAndScissorRect();
			dxCommon->InitializeScissorRect();

			////RootSignatureを設定。PSOに設定しているけど別途設定が必要
			//dxCommon->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
			//dxCommon->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());

			sprite->Draw();


            // 描画処理
            // ここにあなたの描画コードを追加します

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
	delete sprite;

	//winApp_ = nullptr;


	//CloseHandle(fenceEvent);

	//CoUninitialize();
	return 0;
}