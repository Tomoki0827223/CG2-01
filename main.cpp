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

//MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
//	// 1.2.必要な変数の宣言とファイルを開く
//	MaterialData materialData; // 構築するMaterialData
//	std::string line; // ファイルから読んだ1行を格納するもの
//	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
//	assert(file.is_open()); // とりあえず開けなかったら止める
//	// 3.ファイルを読み、MaterialDataを構築
//	while (std::getline(file, line))
//	{
//		std::string identifier;
//		std::istringstream s(line);
//		s >> identifier;
//
//		// identifierに応じた処理
//		if (identifier == "map_Kd") {
//			std::string textureFilename;
//			s >> textureFilename;
//			// 連結してファイルパスにする
//			materialData.textureFilePath = directoryPath + "/" + textureFilename;
//		}
//	}
//	return materialData;
//}
//
//ModelData LoaObjFile(const std::string& directoryPath, const std::string& filename) {
//	
//	// 1. 中で必要となる変数の宣言
//	ModelData modelData; // 構築するModalData
//	std::vector<Vector4> positions; // 位置
//	std::vector<Vector3> normals; // 法線
//	std::vector<Vector2> texcoords; // テクスチャ座標
//	std::string line; // ファイルから読んだ1行を格納するもの
//	
//	// 2. ファイルを開く
//	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
//	assert(file.is_open()); // とりあえず開けなかったら止める
//	
//	// 3. 実際のファイルを読み込み、ModelDataを構築していく
//	while (std::getline(file, line)) {
//		std::string identifier;
//		std::istringstream s(line);
//		s >> identifier; // 先頭の識別子を読む
//		// identifierに応じた処理
//		if (identifier == "v") {
//			Vector4 position;
//			s >> position.x >> position.y >> position.z;
//			position.w = 1.0f;
//
//			position.x *= -1.0f;
//
//			positions.push_back(position);
//		}
//		else if (identifier == "vt") {
//			Vector2 texcoord;
//			s >> texcoord.x >> texcoord.y;
//
//			texcoord.y = 1.0f - texcoord.y;
//			
//			texcoords.push_back(texcoord);
//		}
//		else if (identifier == "vn") {
//			Vector3 normal;
//
//			s >> normal.x >> normal.y >> normal.z;
//
//			normal.x *= -1.0f;
//
//			normals.push_back(normal);
//		}
//		else if (identifier == "f") {
//			
//			VertexData triangle[3];
//			
//			// 面は三角形限定。その他は未対応
//			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
//				std::string vertexDefinition;
//				s >> vertexDefinition;
//
//				// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
//				std::istringstream v(vertexDefinition);
//				uint32_t elementIndeices[3];
//				for (int32_t element = 0; element < 3; ++element) {
//					std::string index;
//					std::getline(v, index, '/'); // 区切りでインデックスを読んでいく
//					elementIndeices[element] = std::stoi(index);
//				}
//
//				// 要素へのIndexから、実際の要素の値をを取得して頂点を構築する
//				Vector4 position = positions[elementIndeices[0] - 1];
//				Vector2 texcoord = texcoords[elementIndeices[1] - 1];
//				Vector3 normal = normals[elementIndeices[2] - 1];
//				VertexData vertex = { position,texcoord,normal };
//				modelData.vertices.push_back(vertex);
//				triangle[faceVertex] = { position,texcoord,normal };
//			}
//
//			// 頂点を逆順で登録することで、回り順を逆にする
//			modelData.vertices.push_back(triangle[2]);
//			modelData.vertices.push_back(triangle[1]);
//			modelData.vertices.push_back(triangle[0]);
//		}
//		else if (identifier == "mtllib") {
//			// mateialTemplateLibraryファイルの名前を取得する
//			std::string materialFilename;
//			s >> materialFilename;
//			// 基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
//			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
//		}
//	}
//	return modelData;
//
//}

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

	bool useMonsterBall = false;

	// テクスチャのロード
	DirectX::ScratchImage mipImages = dxCommon->LoadTexture("resources/monsterBall.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResources = dxCommon->UploadTextureData(textureResource, mipImages);

	// デバッグ出力
	if (textureResource) {
		OutputDebugStringA("Texture loaded successfully.\n");
	}
	else {
		OutputDebugStringA("Failed to load texture.\n");
	}

	// SRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxCommon->GetSRVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxCommon->GetSRVDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	textureSrvHandleCPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	dxCommon->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);

	// デバッグ出力
	OutputDebugStringA("Shader Resource View created.\n");

	TransformVector3 transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

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


			// Sprite用のWorldViewProjectionMatrixを作る
			Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
			Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
			Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(winApp_->kClientWidth), float(winApp_->kClientHeight), 0.0f, 100.0f);
			Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
			transformationMatrixDataSprite->world = worldMatrixSprite;
			transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;

            // GE3
            input->Update();
            // 描画前処理
            dxCommon->PreDraw();
			
			// スプライトにテクスチャを設定
			sprite->SetTexture(textureResource.Get(), textureSrvHandleGPU);

			sprite->Update();

			// ImGuiのフレーム開始
			ImGui_ImplWin32_NewFrame();
			ImGui_ImplDX12_NewFrame();
			ImGui::NewFrame();

			// ImGuiの描画
			ImGui::Render();
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

			dxCommon->InitializeViewportAndScissorRect();
			dxCommon->InitializeScissorRect();

			spriteCommon->CommandListCreate();

			sprite->Draw();

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

	return 0;
}