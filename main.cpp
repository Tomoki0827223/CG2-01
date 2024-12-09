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

#pragma region 単位行列とTransform

//struct TransformVector3
//{
//	Vector3 scale;
//	Vector3 rotate;
//	Vector3 translate;
//
//};
//
//struct VertexData {
//	Vector4 position;
//	Vector2 texcoord;
//	Vector3 nomal;
//};
//
//struct Material
//{
//	Vector4 color;
//	int32_t endleLighting;
//	float padding[3];
//	Matrix4x4 uvTransform;
//};
//
//struct TransformationMatrix
//{
//	Matrix4x4 WVP;
//	Matrix4x4 world;
//
//};
//
//struct DirectionaLight
//{
//	Vector4 color;
//	Vector3 direction;
//	float intensity;
//};
//
//struct Transform1 {
//	Vector3 scale;
//	Vector3 rotate;
//	Vector3 translate;
//};
//
//Transform1 uvTransformSprite{
//	{1.0f, 1.0f, 1.0f},
//	{0.0f, 0.0f, 0.0f},
//	{0.0f, 0.0f, 0.0f},
//};

struct MaterialData {
	std::string textureFilePath;
};


struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

#pragma endregion

MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	// 1.2.必要な変数の宣言とファイルを開く
	MaterialData materialData; // 構築するMaterialData
	std::string line; // ファイルから読んだ1行を格納するもの
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open()); // とりあえず開けなかったら止める
	// 3.ファイルを読み、MaterialDataを構築
	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}
	return materialData;
}

ModelData LoaObjFile(const std::string& directoryPath, const std::string& filename) {
	
	// 1. 中で必要となる変数の宣言
	ModelData modelData; // 構築するModalData
	std::vector<Vector4> positions; // 位置
	std::vector<Vector3> normals; // 法線
	std::vector<Vector2> texcoords; // テクスチャ座標
	std::string line; // ファイルから読んだ1行を格納するもの
	
	// 2. ファイルを開く
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open()); // とりあえず開けなかったら止める
	
	// 3. 実際のファイルを読み込み、ModelDataを構築していく
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // 先頭の識別子を読む
		// identifierに応じた処理
		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;

			position.x *= -1.0f;

			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;

			texcoord.y = 1.0f - texcoord.y;
			
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;

			s >> normal.x >> normal.y >> normal.z;

			normal.x *= -1.0f;

			normals.push_back(normal);
		}
		else if (identifier == "f") {
			
			VertexData triangle[3];
			
			// 面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;

				// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndeices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/'); // 区切りでインデックスを読んでいく
					elementIndeices[element] = std::stoi(index);
				}

				// 要素へのIndexから、実際の要素の値をを取得して頂点を構築する
				Vector4 position = positions[elementIndeices[0] - 1];
				Vector2 texcoord = texcoords[elementIndeices[1] - 1];
				Vector3 normal = normals[elementIndeices[2] - 1];
				VertexData vertex = { position,texcoord,normal };
				modelData.vertices.push_back(vertex);
				triangle[faceVertex] = { position,texcoord,normal };
			}

			// 頂点を逆順で登録することで、回り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib") {
			// mateialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			// 基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	return modelData;

}

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
	sprite->Initialize(dxCommon->GetDevice());

	//ウインドウを表示する
	ShowWindow(winApp_->GetHwnd(), SW_SHOW);

#pragma endregion

	dxCommon->InitializeImGui();
	
	DirectX::ScratchImage mipimage2 = dxCommon->LoadTexture("resources/monsterBall.png");
	const DirectX::TexMetadata& metadata2 = mipimage2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata2);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResources2 = dxCommon->UploadTextureData(textureResource2, mipimage2);

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
			dxCommon->GetCommandList()->SetComputeRootSignature(spriteCommon->GetRootSignature().Get());


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

			//// ImGuiの描画
			//ImGui::Render();
			//ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

			////dxCommon->GetCommandList()->RSSetViewports(1, &viewport);
			////dxCommon->GetCommandList()->RSSetScissorRects(1, &scissorRect);

			//dxCommon->InitializeViewportAndScissorRect();
			//dxCommon->InitializeScissorRect();

			//////RootSignatureを設定。PSOに設定しているけど別途設定が必要
			////dxCommon->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
			////dxCommon->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());

			//spriteCommon->CommonRenderSettings();

			////Sphere
			//dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
			////形状を設定。PSOに設定しているものとはまた別、同じものを設定すると考えておけば良い
			////dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			//dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
			////wvp用のCBufferの場所を設定
			//dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
			//dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
			//dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResorce->GetGPUVirtualAddress());
			////描画

			//dxCommon->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
			////commandList->DrawInstanced(kSubdivision* kSubdivision * 6, 1, 0, 0);

			sprite->Draw(dxCommon->GetCommandList().);

            // 描画後処理
            dxCommon->PostDraw();
        }
    }


	//Windows終了
	winApp_->Finalize();

	delete input;
	delete winApp_;
	delete dxCommon;
	//winApp_ = nullptr;


	//CloseHandle(fenceEvent);

	//CoUninitialize();
	return 0;
}