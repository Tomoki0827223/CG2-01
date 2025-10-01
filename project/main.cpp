#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include "DirectXCommon.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "ModelCommon.h"
#include "Model.h"
#include "ModelManager.h"
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
#include "Camera.h" 
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

	// 1. TextureManagerの初期化
	TextureManager::GetInstance()->Initialize(dxCommon);
	ModelManager::GetInstance()->Initialize(dxCommon);

	ModelManager::GetInstance()->LoadModel("plane.obj"); // 読み込む
	ModelManager::GetInstance()->LoadModel("axis.obj"); // 読み込む
	ModelManager::GetInstance()->LoadModel("multiMesh.obj"); // 読み込む

	// 2. Object3dCommonの生成と初期化
	Object3dCommon* object3dCommon = nullptr;
	object3dCommon = new Object3dCommon();
	object3dCommon->Initialize(dxCommon);

	// --- 【追加】カメラの生成と設定 (スライド「オブジェクトにセットする」) ---
	Camera* camera = new Camera();

	// カメラの初期位置を設定 (例: 後ろに-10.0f移動)
	camera->SetTranslate({ 0.0f, 0.0f, -10.0f });
	// object3dCommonにデフォルトカメラとしてセット
	object3dCommon->SetDefaultCamera(camera);
	// ----------------------------------------------------------------------

	Object3d* object3d = nullptr;
	object3d = new Object3d();
	// Initialize内で Object3dCommon->GetDefaultCamera() が呼ばれ、cameraがセットされる
	object3d->Initialize(object3dCommon);
	object3d->SetModel("multiMesh.obj"); // モデルを設定
	object3d->SetTranslate({ -3.0f, 0.0f, 0.0f });

	// --- 2つ目のオブジェクト（使用例） ---
	Object3d* object3d_2 = nullptr;
	object3d_2 = new Object3d();
	object3d_2->Initialize(object3dCommon);
	object3d_2->SetModel("axis.obj"); // 2つ目のオブジェクトにも同じモデルを設定
	object3d_2->SetTranslate({ 3.0f, 0.0f, 0.0f });
	// ------------------------------------


	// 4. SpriteCommonの生成と初期化
	SpriteCommon* spriteCommon = nullptr;
	spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxCommon);


	// 1. テクスチャをロードする (Objファイルロードの前に実行)
	TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");

	// Spriteの生成
	Sprite* sprite = new Sprite();
	sprite->Initialize(spriteCommon, "resources/uvChecker.png");

	//ウインドウを表示する
	ShowWindow(winApp_->GetHwnd(), SW_SHOW);

#pragma endregion

	bool useMonsterBall = false;
	//Resourcef
	const uint32_t kSubdivision = 36;
	// 複数Sprite生成
	std::vector<Sprite*> sprites;
	const int spriteCount = 5;

	for (int i = 0; i < spriteCount; ++i) {
		Sprite* s = new Sprite();
		const char* texPath = (i % 2 == 0) ? "resources/uvChecker.png" : "resources/monsterBall.png";
		TextureManager::GetInstance()->LoadTexture(texPath);
		s->Initialize(spriteCommon, texPath);
		sprites.push_back(s);
	}

	sprite->position_ = { -0.75f, 0.55f };
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

			camera->Update();

			// ゲーム処理
			// 描画前処理
			dxCommon->PreDraw();

			// ImGuiのフレーム開始
			ImGui_ImplWin32_NewFrame();
			ImGui_ImplDX12_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("Controls");

			// --- [🚨 修正点 2: Camera::Transform へのアクセスを GetTransform() 経由にする 🚨] ---
			// 構造体への直接アクセスを避けるため、GetTransform() を使用する
			Camera::Transform& camTransform = camera->GetTransform();
			ImGui::DragFloat3("Camera Translate", &camTransform.translate.x, 0.1f, -10.0f, 10.0f, "%.2f");
			ImGui::DragFloat3("Camera Rotate (rad)", &camTransform.rotate.x, 0.01f, -6.28f, 6.28f, "%.2f");
			// --------------------------------------------------


			Vector3& rot = object3d->transform.rotate;

			// ImGui::DragFloat3 で回転角度を操作可能にする (rad)
			// rot.x: 参照先（Vector3のx）
			// 0.01f: 変化量（ドラッグ速度）
			ImGui::DragFloat3("Object Rotate (rad)", &rot.x, 0.01f, -6.28f, 6.28f, "%.2f");
			// --------------------------------------------------

			// --- [🚨 修正箇所 2: Sprite の位置操作 🚨] ---
			// 1. 単体のspriteの位置 (position_) を参照
			Vector2& pos = sprite->position_;
			std::string label = "Sprite Position";

			// 2. ImGui::DragFloat2 で位置を操作可能にする (NDC座標系)
			ImGui::DragFloat2(label.c_str(), &pos.x, 0.01f, -1.0f, 1.0f, "%.2f");
			ImGui::End(); // 👈 ImGuiブロックの終了

			// ImGui描画
			ImGui::Render();
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

			// --- [🚨 ImGui後の設定リセット 🚨] ---
			dxCommon->InitializeViewportAndScissorRect(); // ビューポートとシザー矩形の設定値を更新
			dxCommon->InitializeScissorRect();
			
			// コマンドリストにビューポートとシザー矩形を再設定
			// ----------------------------------------------------

			// 3Dオブジェクトの描画準備 (RootSignature/PipelineStateを設定)
			object3dCommon->SetCommand();

			// 3Dオブジェクト個々の描画
			object3d->Update();
			object3d->Draw();

			// 2つ目のオブジェクトを描画
			object3d_2->Update();
			object3d_2->Draw();

			// 2D（Sprite）の描画準備 (3D描画後に行う)
			spriteCommon->CommandListCreate();

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
	ModelManager::GetInstance()->Finalize();

	delete input;
	delete winApp_;
	delete dxCommon;
	delete spriteCommon;
	delete object3dCommon;
	delete object3d;
	delete camera;
	delete object3d_2;

	return 0;
}