#include "Object3dCommon.h"
#include "DirectXCommon.h"
#include "Logger.h" // Logのために必要
#include <cassert>

// DirectXTex/d3dx12.hのインクルードは通常不要ですが、
// SpriteCommonのロジックが依存している場合は追加が必要です。
// ここでは必要最低限のヘッダーのみとしています。

using namespace Microsoft::WRL;

void Object3dCommon::Initialize(DirectXCommon* dxCommon)
{
	// 引数で受け取ってメンバ変数に記録する
	dxCommon_ = dxCommon;

	// グラフィックスパイプラインの生成を実行
	CreatePipelineState();
}

// --- ルートシグネチャ作成 ---
void Object3dCommon::CreateRootSignature()
{
	// ルートシグネチャの定義 (SpriteCommon.cppのコードを3D向けに調整)
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// DescriptorRange (SRV用)
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0; // t0 レジスタ
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// RootParameter (4つ: マテリアル(CBV), 変換行列(CBV), テクスチャ(SRV), Light(CBV))
	D3D12_ROOT_PARAMETER rootParameters[4] = {};

	// 0: マテリアル (色、ライティング有効フラグなど) 用のCBV (b0) - ピクセルシェーダー
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	// 1: 変換行列 (WVP, World) 用のCBV (b0) - 頂点シェーダー
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	// 2: テクスチャ (SRV) 用のDescriptorTable (t0) - ピクセルシェーダー
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	// 3: Light (DirectionalLightなど) 用のCBV (b1) - ピクセルシェーダー
	// ※ SpriteCommonではUVTransform用のCBV (b1)だったが、ここではLight用に再利用
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].Descriptor.ShaderRegister = 1;

	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	// Samplerの定義
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0; // s0 レジスタ
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// ルートシグネチャのシリアライズと生成
	ComPtr<ID3DBlob> signatureBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

	if (FAILED(hr))
	{
		// エラーログを出力
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
}

// --- パイプラインステート生成 ---
void Object3dCommon::CreatePipelineState()
{
	// ルートシグネチャはパイプラインステートの初期化に必要なので、最初に呼び出す
	CreateRootSignature();

	// 頂点インプットレイアウトの定義 (3DオブジェクトはPosition, Texcoord, Normalの3要素)
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// ブレンド設定 (SpriteCommonからコピー、ここでは不透明描画を想定)
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// ラスタライザ設定 (SpriteCommonからコピー、裏面カリング)
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// シェーダーのコンパイル
	// 1. **シェーダーファイル**のパスを3D用に変更
	ComPtr<IDxcBlob> vertexShaderBlob = dxCommon_->compileShader(L"resources/shaders/Object3D.VS.hlsl", L"vs_6_0");
	ComPtr<IDxcBlob> pixelShaderBlob = dxCommon_->compileShader(L"resources/shaders/Object3D.PS.hlsl", L"ps_6_0");
	assert(vertexShaderBlob != nullptr);
	assert(pixelShaderBlob != nullptr);

	// 深度ステンシル設定 (3D描画のため深度テストを有効に)
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// パイプライン生成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPinpelineStateDesc{};
	graphicsPinpelineStateDesc.pRootSignature = rootSignature.Get();
	graphicsPinpelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPinpelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
	graphicsPinpelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
	graphicsPinpelineStateDesc.BlendState = blendDesc;
	graphicsPinpelineStateDesc.RasterizerState = rasterizerDesc;
	graphicsPinpelineStateDesc.DepthStencilState = depthStencilDesc; // 深度設定を適用

	graphicsPinpelineStateDesc.NumRenderTargets = 1;
	graphicsPinpelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	graphicsPinpelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	graphicsPinpelineStateDesc.SampleDesc.Count = 1;
	graphicsPinpelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	graphicsPinpelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT; // DSVフォーマット

	// パイプラインステートの生成
	HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPinpelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
}

// --- 共通描画設定 (毎フレーム) ---
void Object3dCommon::SetCommand()
{
	// 毎フレーム、3Dオブジェクトの描画前に呼び出す共通設定
	auto commandList = dxCommon_->GetCommandList();

	// ルートシグネチャをセットするコマンド
	commandList->SetGraphicsRootSignature(rootSignature.Get());

	// グラフィックスパイプラインステートをセットするコマンド
	commandList->SetPipelineState(graphicsPipelineState.Get());

	// プリミティブトポロジーをセットするコマンド (3Dオブジェクトは通常TRIANGLELIST)
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}