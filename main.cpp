#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include "DirectXCommon.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "TextureManager.h"
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

	TextureManager::GetInstance()->Initialize(dxCommon);
	// 1. テクスチャをロードする
	TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");

	// Spriteの生成
	Sprite* sprite = new Sprite();
	sprite->Initialize(spriteCommon, "resources/uvChecker.png");


	//ウインドウを表示する
	ShowWindow(winApp_->GetHwnd(), SW_SHOW);

#pragma endregion

	bool useMonsterBall = false;

	TransformVector3 transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	//Resourcef
	const uint32_t kSubdivision = 36;


	//モデル読み込み
	ModelData modelData = LoaObjFile("resources", "axis.obj");

	// 複数Sprite生成
	std::vector<Sprite*> sprites;
	const int spriteCount = 5;
	const float startX = -0.7f; // NDC座標系で左端
	const float startY = 0.3f;  // NDC座標系で上
	const float offsetX = 0.35f; // NDC座標系で横方向の間隔

	for (int i = 0; i < spriteCount; ++i) {
		Sprite* s = new Sprite();
		const char* texPath = (i % 2 == 0) ? "resources/uvChecker.png" : "resources/monsterBall.png";
		TextureManager::GetInstance()->LoadTexture(texPath);
		s->Initialize(spriteCommon, texPath);
		sprites.push_back(s);
	}

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

			ImGui::Begin("Sprite Controls");
			for (int i = 0; i < sprites.size(); ++i) {
				Vector2& pos = sprites[i]->position_;
				// ラベルを動的に生成
				std::string label = "Sprite " + std::to_string(i) + " Position";
				ImGui::DragFloat2(label.c_str(), &pos.x, 0.01f, -1.0f, 1.0f, "%.2f");
			}
			ImGui::End();

			// ImGui描画
			ImGui::Render();
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());
			
			spriteCommon->CommandListCreate();

			dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(sprite->textureIndex));

			dxCommon->InitializeViewportAndScissorRect();
			dxCommon->InitializeScissorRect();

			// 描画ループ
			for (auto s : sprites) {
				// ここで各スプライトのテクスチャSRVをセット
				dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(s->textureIndex));
				s->Update();
				s->Draw();
			}

			// 例: スペースキーでテクスチャ切り替え
			if (input->TriggerKey(DIK_SPACE)) {
				sprite->ChangeTexture("resources/monsterBall.png");
			}

			// 1枚目のspriteも同様に
			dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(sprite->textureIndex));
			sprite->Update();
			sprite->Draw();

            // 描画後処理
            dxCommon->PostDraw();
        }
    }


	//Windows終了
	winApp_->Finalize();
	TextureManager::GetInstance()->Finalize();

	delete input;
	delete winApp_;
	delete dxCommon;
	delete spriteCommon;

	return 0;
}