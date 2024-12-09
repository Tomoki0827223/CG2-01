#include "Sprite.h"
#include <cstring>
#include "affine.h"
#include "externals/DirectXTex/d3dx12.h"

Sprite::Sprite() {}

Sprite::~Sprite() {
    // リソースのクリーンアップ
    if (vertexResource) vertexResource->Unmap(0, nullptr);
    if (indexResource) indexResource->Unmap(0, nullptr);
    if (transformationMatrixResource) transformationMatrixResource->Unmap(0, nullptr);
}

void Sprite::Initialize(SpriteCommon* spriteCommon) {
    spriteCommon_ = spriteCommon;

    // 頂点データとインデックスデータを作成
    CreateVertexData(spriteCommon->GetDirectXCommon()->GetDevice());
    CreateIndexData(spriteCommon);

    // 座標変換行列データを作成
    CreateTransformationMatrix(spriteCommon->GetDirectXCommon()->GetDevice());

    // 頂点バッファビューの設定
    vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(VertexData);
    vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;

    // インデックスバッファビューの設定
    indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
    indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
    indexBufferView.Format = DXGI_FORMAT_R32_UINT;
}

void Sprite::Update() {
    // 頂点リソースにデータを書き込む (4点分)
    VertexData vertices[] = {
        { { -0.5f,  0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
        { {  0.5f,  0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
        { { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
        { {  0.5f, -0.5f, 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } }
    };

    std::memcpy(vertexData, vertices, sizeof(vertices));

    // インデックスリソースにデータを書き込む (6個分)
    uint32_t indices[] = { 0, 1, 2, 2, 1, 3 };
    std::memcpy(indexData, indices, sizeof(indices));

    // Transform 情報を作る
    // (ここでは例として、単位行列を使用しています。必要に応じて変換情報を設定してください)
    Matrix4x4 worldMatrix = MakeIdentity4x4();

    // ViewMatrix を作って単位行列を代入
    Matrix4x4 viewMatrix = MakeIdentity4x4();

    // ProjectionMatrix を作って並行投影行列を書き込む
    Matrix4x4 projectionMatrix = MakeOrthographicMatrix(
        -1.0f, 1.0f, // 左右
        -1.0f, 1.0f, // 上下
        0.0f, 100.0f // ニアクリップとファークリップ
    );

    // WVP 行列を計算して transformationMatrixData に書き込む
    transformationMatrixData->WVP = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

    // World 行列を transformationMatrixData に書き込む
    transformationMatrixData->World = worldMatrix;
}

void Sprite::Draw() {
    // VertexBufferView を設定
    spriteCommon_->GetDirectXCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);

    // IndexBufferView を設定
    spriteCommon_->GetDirectXCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView);

    // マテリアル CBuffer の場所を設定
    if (materialResource) {
        D3D12_GPU_VIRTUAL_ADDRESS materialAddress = materialResource->GetGPUVirtualAddress();
        spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, materialAddress); // ルートパラメータ 1 にバインド
    }

    // 座標変換行列 CBuffer の場所を設定
    if (transformationMatrixResource) {
        D3D12_GPU_VIRTUAL_ADDRESS transformAddress = transformationMatrixResource->GetGPUVirtualAddress();
        spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, transformAddress); // ルートパラメータ 0 にバインド
    }

    // SRV の Descriptor Table の先頭を設定
    if (spriteCommon_->GetDirectXCommon()->GetSRVDescriptorHeap()) {
        CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(spriteCommon_->GetDirectXCommon()->GetSRVDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
        spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, srvHandle); // ルートパラメータ 2 にバインド
    }

    // 描画! (DrawCall / ドローコール)
    spriteCommon_->GetDirectXCommon()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // トポロジーを設定
    spriteCommon_->GetDirectXCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0); // 6つのインデックスで 1 インスタンスを描画
}

void Sprite::CreateVertexData(SpriteCommon* spriteCommon) {
    // 頂点データを定義
    VertexData vertices[] = {
        { { -0.5f,  0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
        { {  0.5f,  0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
        { { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
        { {  0.5f, -0.5f, 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } }
    };

    auto vertexBufferSize = sizeof(vertices);

    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = vertexBufferSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    spriteCommon_->GetDirectXCommon()->CreateCommittedResource(
        &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource)
    );

    vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    std::memcpy(vertexData, vertices, vertexBufferSize);
    vertexResource->Unmap(0, nullptr);
}

void Sprite::CreateTransformationMatrix(SpriteCommon* spriteCommon) {
    // バッファサイズを計算
    auto bufferSize = (sizeof(TransformationMatrix) + 255) & ~255; // 256バイトアラインメント

    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = bufferSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    spriteCommon_->CreateCommittedResource(
        &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&transformationMatrixResource)
    );

    transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
    transformationMatrixResource->Unmap(0, nullptr);
}
