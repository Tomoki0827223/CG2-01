#include "Sprite.h"
#include "SpriteCommon.h"
#include "WinApp.h"

void Sprite::Initialize(SpriteCommon* spriteCommon)
{
	this->spriteCommon_ = spriteCommon;

	//頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexReComPtr = spriteCommon->GetDXCommon()->CreateBufferResource(sizeof(VertexData));
	vertexResource = vertexReComPtr.Get();

	Microsoft::WRL::ComPtr<ID3D12Resource> indexReComPtr = spriteCommon->GetDXCommon()->CreateBufferResource(sizeof(uint32_t) * 6);
	indexResource = indexReComPtr.Get();


	vertexBufferView.SizeInBytes = sizeof(VertexData);
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();// リソースの先頭のアドレスから使う
	vertexBufferView.StrideInBytes = sizeof(VertexData);// 1頂点あたりのサイズ

	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	//頂点リソースにデータを書き込む
	vertexData = nullptr;
	//書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	//頂点データを書き込む
	indexData = nullptr;
	//書き込むためのアドレスを取得
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	//Sprite用のTransfomationMatrix用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = spriteCommon->GetDXCommon()->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* wvpDeta = nullptr;
	//書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpDeta));
	//単位行列を書き込んでおく
	wvpDeta->world = MakeIdentity4x4();
	wvpDeta->WVP = MakeIdentity4x4();

	//vetexResourceSprite頂点バッファーを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{ };
	//リソースの先頭のアドレスから使う
	vertexBufferViewSprite.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
	//1頂点当たりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);
	//頂点リソースにデータを書き込む
	VertexData* vertexDataSprite = nullptr;
	//書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));

	vertexData[0].position = { 0.0f, 360.0f, 0.0f, 1.0f }; // 左下
	vertexData[0].texcoord = { 0.0f, 1.0f };
	vertexData[0].normal = { 0.0f, 0.0f, -1.0f };

	vertexData[1].position = { 0.0f, 0.0f, 0.0f, 1.0f }; // 左上
	vertexData[1].texcoord = { 0.0f, 0.0f };
	vertexData[1].normal = { 0.0f, 0.0f, -1.0f };

	vertexData[2].position = { 640.0f, 360.0f, 0.0f, 1.0f }; // 右下
	vertexData[2].texcoord = { 1.0f, 1.0f };
	vertexData[2].normal = { 0.0f, 0.0f, -1.0f };

	vertexData[3].position = { 640.0f, 0.0f, 0.0f, 1.0f }; // 右上
	vertexData[3].texcoord = { 1.0f, 0.0f };
	vertexData[3].normal = { 0.0f, 0.0f, -1.0f };


	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 1;
	indexData[4] = 3;
	indexData[5] = 2;

	//Material用のResourceを作る
	materialResource = spriteCommon->GetDXCommon()->CreateBufferResource(sizeof(Material));
	materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	////こここで色かえられるよ
	materialData->color = Vector4{ 1.0f,1.0f,1.0f,1.0f };
	materialData->endleLighting = false;
	materialData->uvTransform = MakeIdentity4x4();

	//TransformationMatrixResource
	transformationMatrixResource = spriteCommon->GetDXCommon()->CreateBufferResource(sizeof(TransformationMatrix));
	transformationMatrixData = nullptr;
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->world = MakeIdentity4x4();
}

void Sprite::Update()
{
	// スプライト用のWorldViewProjectionMatrixを作成
	Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
	Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
	Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(winApp_->kClientWidth), float(winApp_->kClientHeight), 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));

	// 行列をTransformationMatrixに設定
	transformationMatrixData->world = worldMatrixSprite;
	transformationMatrixData->WVP = worldViewProjectionMatrixSprite;
}

void Sprite::Draw()
{
	// Spriteの描画。変更が必要なものだけ変更する
	spriteCommon_->GetDXCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定
	spriteCommon_->GetDXCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView);// IBVを設定//06_00

	spriteCommon_->GetDXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// TransformationMatrixCBufferの場所を設定
	spriteCommon_->GetDXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

	spriteCommon_->GetDXCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, spriteCommon_->GetDXCommon()->GetSRVGPUDescriptorHandle(1));

	// 描画！（DrawCall/ドローコール）6個のインデックスを使用し1つのインスタンスを描画。その他は当面0で良い
	spriteCommon_->GetDXCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);//06_00
}
