#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include "DirectXCommon.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "SrvManager.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "ModelCommon.h"
#include "Model.h"
#include "ModelManager.h"
#include "ParticleEmitter.h"
#include "Particle.h"
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


	// ImGuiで制御するパーティクルエミッタのパラメータ
	Vector3 emitterPosition = { 0.0f, 5.0f, 0.0f };
	int emitCount = 5;
	float speedMin = 2.0f;
	float speedMax = 5.0f;
	float lifeTimeMin = 1.0f;
	float lifeTimeMax = 3.0f;

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

	// SRVManagerの生成と初期化
	SrvManager* srvManager = nullptr; // ★追加
	srvManager = new SrvManager(); // ★追加
	srvManager->Initialize(dxCommon); // ★追加

	// 1. TextureManagerの初期化 (srvManagerを渡す)
	TextureManager::GetInstance()->Initialize(dxCommon, srvManager); // ★変更：srvManagerを渡す
	// ModelManagerのInitializeも同様にsrvManagerが必要な場合は変更が必要
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

	// ModelManagerとDirectXCommonがシングルトンパターンなどでアクセス可能と仮定します。
	// ModelManagerインスタンスを取得 (シングルトンと仮定)
	ModelManager* modelManager = ModelManager::GetInstance();

	// --------------------------------------------------------------------------------
	// 【修正箇所】パーティクル用モデルのロードと取得
	// --------------------------------------------------------------------------------

	// 1. ロード前のモデル数を取得 (ModelManager.hに GetModelSize() の追加が必要です)
	size_t preLoadSize = modelManager->GetModelSize(); // error C2039, C2065 対策

	// 2. パーティクル用のモデルをロード (Loadはvoidを返す)
	modelManager->Load("plane.obj");

	// 3. ロードしたモデルをModelManagerから取得
	uint32_t particleModelIndex = static_cast<uint32_t>(preLoadSize);
	Model* particleModel = modelManager->GetModel(particleModelIndex); // error C2039 対策

	// --------------------------------------------------------------------------------

	// ParticleEmitterのインスタンス化
	ParticleEmitter emitter;

	// Object3dCommon* object3dCommon = Object3dCommon::GetInstance(); // <--- この行を削除またはコメントアウト
	// Camera* mainCamera = camera;                                     // <--- この行を削除またはコメントアウト

	// Emitterの初期化
	// object3dCommonとcameraは既存のコードで既に宣言・初期化されている変数を使う
	emitter.Initialize(object3dCommon, particleModel, camera); // error C2374, C2086 対策

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
			dxCommon->PreDraw(); // RTV/DSVの設定のみ
			// srvManager->PreDraw(); // 1回目のSrvManagerヒープ設定は削除 (ImGuiの描画前にはdxCommonヒープが必要なため)
			
			// === ImGui処理の開始 ===
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			// ImGui ウィンドウの作成 (ここはループ内に残す)
			ImGui::Begin("Particle Emitter Settings");

			// 変数へのポインタを渡すことで、ImGuiがその値を変更します
			ImGui::DragFloat3("Position (X, Y, Z)", &emitterPosition.x, 0.1f, -10.0f, 10.0f); //
			ImGui::SliderInt("Emit Count", &emitCount, 0, 50);
			ImGui::DragFloatRange2("Min/Max Speed", &speedMin, &speedMax, 0.1f, 0.0f, 20.0f, "Min: %.1f", "Max: %.1f");
			ImGui::DragFloatRange2("Min/Max Life Time", &lifeTimeMin, &lifeTimeMax, 0.1f, 0.1f, 10.0f, "Min: %.1f s", "Max: %.1f s");

			ImGui::End();

			// === ImGuiの描画データ生成 ===
			ImGui::Render();
			// ------------------------------------

			// --- 【修正 1】ImGuiが使うヒープを設定 ---
			// ImGuiのフォントテクスチャはDirectXCommonのメインSRVヒープ(srvDescriptorHeap)に確保されている
			{
				ID3D12DescriptorHeap* descriptorHeaps[] = { dxCommon->GetSRVDescriptorHeap().Get() };
				dxCommon->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
			}

			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

			// --- [🚨 ImGui後の設定リセット 🚨] ---
			dxCommon->InitializeViewportAndScissorRect();
			dxCommon->InitializeScissorRect();
			// ----------------------------------------------------

			// 3Dオブジェクトの描画準備 (RootSignature/PipelineStateを設定)
			object3dCommon->SetCommand();

			// 【修正 2】SrvManagerヒープに切り替える
			srvManager->PreDraw(); // ★SrvManagerのヒープに切り替える

			// 3Dオブジェクト個々の描画
			object3d->Update();
			object3d->Draw();

			// 2つ目のオブジェクトを描画
			object3d_2->Update();
			object3d_2->Draw();

			// --- 更新処理 ---
			// 毎フレーム、または特定のタイミングでパーティクルを生成
			emitter.Emit(
				emitterPosition,       // 永続的な変数を使用
				emitCount,             // 永続的な変数を使用
				speedMin, speedMax,    // 永続的な変数を使用
				lifeTimeMin, lifeTimeMax // 永続的な変数を使用
			);

			// パーティクルシステムの更新
			emitter.Update();
			// パーティクルの描画
			emitter.Draw();

			// 2D（Sprite）の描画準備 (3D描画後に行う)
			spriteCommon->CommandListCreate();

			// 【修正 3】Spriteの描画前にもヒープ設定を維持 (ただし、既にSrvManagerヒープなので必須ではないが安全のため残す)
			// srvManager->PreDraw(); // 既に上で行っているので、この行は削除またはコメントアウトしても良い

			// 例: スペースキーでテクスチャ切り替え
			if (input->TriggerKey(DIK_SPACE)) {
				sprite->ChangeTexture("resources/monsterBall.png");
			}

			// 1枚目のspriteも同様に
			dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPUByFilePath(sprite->filePath_));
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
	delete srvManager;

	return 0;
}