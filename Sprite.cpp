#include "Sprite.h"
#include "SpriteCommon.h"

Sprite::~Sprite()
{
    delete sprite;
}

void Sprite::Initialize(SpriteCommon* spriteCommon)
{

}

void Sprite::Update()
{
}

void Sprite::Draw()
{
}

//void Sprite::Initialize(SpriteCommon* spriteCommon)
//{
//    this->spriteCommon_ = spriteCommon;
//
//    // 頂点リソースを作成する
//    vertexResource = spriteCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(VertexData) * 4);
//
//    // インデックスリソースを作成する
//    indexResource = spriteCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(uint32_t) * 6);
//
//    // 頂点バッファビューを作成する
//    vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
//    vertexBufferView.StrideInBytes = sizeof(VertexData); // 頂点データの構造体サイズ
//    vertexBufferView.SizeInBytes = sizeof(VertexData) * 4; // 全頂点データのサイズ
//
//    // インデックスバッファビューを作成する
//    indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
//    indexBufferView.SizeInBytes = sizeof(uint32_t) * 6; // 全インデックスデータのサイズ
//
//    // 頂点リソースにデータを書き込むためのアドレスを取得
//    vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
//
//    // 頂点データを設定
//    vertexData[0] = { { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } }; // 左下
//    vertexData[1] = { { 0.5f, -0.5f, 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } };  // 右下
//    vertexData[2] = { { -0.5f, 0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };  // 左上
//    vertexData[3] = { { 0.5f, 0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };   // 右上
//
//    vertexResource->Unmap(0, nullptr);
//
//    // インデックスリソースにデータを書き込むためのアドレスを取得
//    indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
//
//    // インデックスデータを設定
//    indexData[0] = 0; indexData[1] = 1; indexData[2] = 2; // 三角形1
//    indexData[3] = 2; indexData[4] = 1; indexData[5] = 3; // 三角形2
//
//    indexResource->Unmap(0, nullptr);
//
//    //Material用のResourceを作る
//    materialResource = spriteCommon->GetDirectXCommon()->CreateBufferResource(sizeof(Material));
//    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
//
//	//マテリアルデータの初期値を設定
//	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
//	materialData->endleLighting = 1;
//	materialData->uvTransform = MakeIdentity4x4();
//
//    //TransformationMatrixResource
//    transformationMatrixResource = spriteCommon->GetDirectXCommon()->CreateBufferResource(sizeof(TransformationMatrix));
//	//バッファリソース内のデータを指すポインタ
//	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrix));
//
//	transformationMatrix->WVP = MakeIdentity4x4();
//	transformationMatrix->world = MakeIdentity4x4();
//}
//
//// Update method
//void Sprite::Update()
//{
//    // 頂点リソースにデータを書き込む
//    VertexData* mappedVertexData = nullptr;
//    if (SUCCEEDED(vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedVertexData))))
//    {
//        mappedVertexData[0] = { { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } }; // 左下
//        mappedVertexData[1] = { { 0.5f, -0.5f, 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } };  // 右下
//        mappedVertexData[2] = { { -0.5f, 0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };  // 左上
//        mappedVertexData[3] = { { 0.5f, 0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };   // 右上
//        vertexResource->Unmap(0, nullptr);
//    }
//
//    // インデックスリソースにデータを書き込む
//    uint32_t* mappedIndexData = nullptr;
//    if (SUCCEEDED(indexResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedIndexData))))
//    {
//        mappedIndexData[0] = 0; mappedIndexData[1] = 1; mappedIndexData[2] = 2; // 三角形1
//        mappedIndexData[3] = 2; mappedIndexData[4] = 1; mappedIndexData[5] = 3; // 三角形2
//        indexResource->Unmap(0, nullptr);
//    }
//
//    // Transform 情報を作成
//    TransformVector3 transform = {}; // Transform 構造体を作成（位置、回転、スケールなどを設定）
//    transform.translate = { 0.0f, 0.0f, 0.0f };
//    transform.rotate = { 0.0f, 0.0f, 0.0f };
//    transform.scale = { 1.0f, 1.0f, 1.0f };
//
//    // ワールド行列を作成
//    Matrix4x4 worldMatrix = CreateWorldMatrix(transform);
//
//    // ビュー行列を単位行列として初期化
//    Matrix4x4 viewMatrix = MakeIdentity4x4();
//
//    // プロジェクション行列を作成（並行投影）
//    Matrix4x4 projectionMatrix = MakeOrthographicMatrix(
//        -1.0f, 1.0f,  // 左右クリップ平面
//        -1.0f, 1.0f,  // 上下クリップ平面
//        0.0f, 100.0f  // 近遠クリップ平面
//    );
//
//    // ワールド、ビュー、プロジェクションを合成
//    Matrix4x4 wvpMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
//
//    // TransformationMatrixData 構造体に書き込む
//    if (transformationMatrixResource)
//    {
//        transformationMatrix->WVP = wvpMatrix;
//        transformationMatrix->world = worldMatrix;
//    }
//}
//
//void Sprite::Draw()
//{
//    // DirectX コマンドリストを取得
//    ID3D12GraphicsCommandList* commandList = spriteCommon_->GetDirectXCommon()->GetCommandList();
//
//    // 頂点バッファビューを設定
//    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
//    // インデックスバッファビューを設定
//    commandList->IASetIndexBuffer(&indexBufferView);
//
//    // マテリアルの CBuffer (定数バッファ) を設定
//    commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
//
//    D3D12_GPU_DESCRIPTOR_HANDLE materialHandle = materialHeap->GetGPUDescriptorHandleForHeapStart();
//    commandList->SetGraphicsRootDescriptorTable(0, materialHandle); // ルートパラメータ 0 にバインド
//
//    // 座標変換行列の CBuffer を設定
//    commandList->SetGraphicsRootConstantBufferView(0, transformationMatrixResource->GetGPUVirtualAddress());
//
//    // SRV (シェーダリソースビュー) の Descriptor Table を設定
//    ID3D12DescriptorHeap* srvHeap = spriteCommon_->GetDirectXCommon()->GetSRVDescriptorHeap().Get();
//    commandList->SetDescriptorHeaps(1, &srvHeap);
//
//    
//    D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();
//    commandList->SetGraphicsRootDescriptorTable(2, srvHandle); // ルートパラメータ 2 にバインド
//
//    // 描画 (DrawCall)
//    commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
//}
