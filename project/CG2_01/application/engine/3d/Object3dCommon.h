#pragma once
#include <wrl.h>
#include <d3d12.h>

// 前方宣言
class DirectXCommon;

class Camera; // Cameraクラスの前方宣言
class Object3dCommon
{
public:

	// --- [初期化] ---
	// 引数でDirectXCommonのポインタを受け取り、メンバに記録する
	void Initialize(DirectXCommon* dxCommon); //

	// --- [Getter] ---
	// DirectXCommonへのポインタを返す
	DirectXCommon* GetDXCommon() const { return dxCommon_; } //

	// --- [共通描画設定] ---
	// 毎フレーム、3Dオブジェクト個別の描画前に呼び出す描画ルール設定
	void SetCommand(); // スライドの「共通描画設定()」に相当

	// --- 【追加】デフォルトカメラのSetter/Getter (スライド「デフォルトカメラ」) ---
	void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	Camera* GetDefaultCamera() const { return defaultCamera; }

private:

	// --- [依存と共通データ] ---
	// DirectXCommonのポインタを保持
	DirectXCommon* dxCommon_ = nullptr; //

	// ルートシグネチャとパイプラインステート
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

	// --- [内部処理関数] ---
	// ルートシグネチャの作成とパイプラインの生成を関数化
	void CreateRootSignature(); // ルートシグネチャの作成() に相当
	void CreatePipelineState(); // グラフィックスパイプラインの生成() に相当

	Camera* defaultCamera = nullptr;
};