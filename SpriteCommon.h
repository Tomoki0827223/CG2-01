#pragma once
#include "externals/DirectXTex/d3dx12.h"
#include <cassert>
#include "Logger.h"
#include "DirectXCommon.h"

class SpriteCommon
{
public:

	~SpriteCommon();

	void Initialize(DirectXCommon* dxCommon);

	// 共通描画設定 
	void CommonRenderSettings();

	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() { return rootSignature_; }
	//Microsoft::WRL::ComPtr<ID3D12PipelineState> GetGraphicsPipelineState() { return graphicsPipelineState_; }

	DirectXCommon* GetDirectXCommon() { return dxCommon_; }

private:

	void CreateRootSignature();
	void graphicsPipelineState();

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
	//Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_ = nullptr;

	SpriteCommon* spriteCommon = nullptr;
	DirectXCommon* dxCommon_ = nullptr;

};