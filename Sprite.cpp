#include "Sprite.h"
#include "externals/DirectXTex/d3dx12.h"

void Sprite::Initialize(SpriteCommon* spriteCommon)
{
    this->spriteCommon_ = spriteCommon;

    // VertexResourceを作る
    D3D12_RESOURCE_DESC vertexResourceDesc = {};
    vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vertexResourceDesc.Width = sizeof(Vector3) * vertexCount_;
    vertexResourceDesc.Height = 1;
    vertexResourceDesc.DepthOrArraySize = 1;
    vertexResourceDesc.MipLevels = 1;
    vertexResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    vertexResourceDesc.SampleDesc.Count = 1;
    vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
    HRESULT hr = spriteCommon_->GetDXCommon()->GetDevice()->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &vertexResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertexBuffer_));
    assert(SUCCEEDED(hr));

    // IndexResourceを作る
    D3D12_RESOURCE_DESC indexResourceDesc = {};
    indexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    indexResourceDesc.Width = sizeof(uint16_t) * indexCount_;
    indexResourceDesc.Height = 1;
    indexResourceDesc.DepthOrArraySize = 1;
    indexResourceDesc.MipLevels = 1;
    indexResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    indexResourceDesc.SampleDesc.Count = 1;
    indexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    hr = spriteCommon_->GetDXCommon()->GetDevice()->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &indexResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&indexBuffer_));
    assert(SUCCEEDED(hr));

    // vertexBufferView_を作る(値を設定するだけ)
    vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<UINT>(vertexResourceDesc.Width);
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

    // indexBufferView_を作る(値を設定するだけ)
    indexBufferView_.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
    indexBufferView_.SizeInBytes = static_cast<UINT>(indexResourceDesc.Width);
    indexBufferView_.Format = DXGI_FORMAT_R16_UINT;

    // VertexResourceにデータを書き込むだけのアドレスを取得して、vertexData_に割り当てる
    hr = vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
    assert(SUCCEEDED(hr));

    // IndexResourceにデータを書き込むだけのアドレスを取得して、indexData_に割り当てる
    hr = indexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
    assert(SUCCEEDED(hr));

    // MaterialResourceを作る
    D3D12_RESOURCE_DESC materialResourceDesc = {};
    materialResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    materialResourceDesc.Width = sizeof(Material);
    materialResourceDesc.Height = 1;
    materialResourceDesc.DepthOrArraySize = 1;
    materialResourceDesc.MipLevels = 1;
    materialResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    materialResourceDesc.SampleDesc.Count = 1;
    materialResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    hr = spriteCommon_->GetDXCommon()->GetDevice()->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &materialResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&materialBuffer_));
    assert(SUCCEEDED(hr));

    // materialData_にMaterialResourceにデータを書き込むだけのアドレスを取得して割り当てる
    hr = materialBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
    assert(SUCCEEDED(hr));

    //マテリアルデータの初期値を書き込む
    materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData_->enableLighting = false;
	materialData_->uvTransform = MakeIdentity4x4();

	//座標変換行列リソースを作る
	D3D12_RESOURCE_DESC transformationMatrixResourceDesc = {};
	transformationMatrixResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    transformationMatrixResourceDesc.Width = sizeof(TransfomaitionMatrix);
	transformationMatrixResourceDesc.Height = 1;
    transformationMatrixResourceDesc.DepthOrArraySize = 1;
    transformationMatrixResourceDesc.MipLevels = 1;
    transformationMatrixResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    transformationMatrixResourceDesc.SampleDesc.Count = 1;
    transformationMatrixResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//座標変換行列にデータを書き込むだけのアドレスを取得して、transformationMatrixData_に割り当てる
	hr = spriteCommon_->GetDXCommon()->GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&transformationMatrixResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&transformationMatrixBuffer_));
	assert(SUCCEEDED(hr));

	hr = transformationMatrixBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	assert(SUCCEEDED(hr));

	//単位行列を書き込んでおく
    transformationMatrixData_->WVP = MakeIdentity4x4();
	transformationMatrixData_->world = MakeIdentity4x4();

}

void Sprite::SetTexture(ID3D12Resource* texture, D3D12_GPU_DESCRIPTOR_HANDLE srvHandle) {
    texture_ = texture;
    srvHandle_ = srvHandle;
}


void Sprite::Update()
{
	//頂点データを書き込む
    vertexData_[0].position = Vector4(-0.5f, 0.5f, 0.0f, 1.0f); // 左上
    vertexData_[1].position = Vector4(0.5f, 0.5f, 0.0f, 1.0f);  // 右上
    vertexData_[2].position = Vector4(-0.5f, -0.5f, 0.0f, 1.0f); // 左下
    vertexData_[3].position = Vector4(0.5f, -0.5f, 0.0f, 1.0f);  // 右下

	//インデックスリソースにデータを書き込む(六個分)
	indexData_[0] = 0;
	indexData_[1] = 1;
	indexData_[2] = 2;
	indexData_[3] = 1;
	indexData_[4] = 3;
	indexData_[5] = 2;

    //Transform情報を作る
	Matrix4x4 worldMatrix = MakeIdentity4x4();
	Matrix4x4 view = MakeIdentity4x4();
	Matrix4x4 projection = MakeIdentity4x4();
    Matrix4x4 wvp = Multiply(worldMatrix, Multiply(view, projection));

    //TransfromからWorldMatrixを作る
	transformationMatrixData_->world = worldMatrix;
	transformationMatrixData_->WVP = wvp;

	//ViewMatrixを作って単位行列を代入
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	
	//ProjectionMatrixを作って並行投影行列を書き込む
    transformationMatrixData_->WVP = Multiply(worldMatrix, Multiply(viewMatrix, projection));
    transformationMatrixData_->world = worldMatrix;
}


void Sprite::Draw() {
    ID3D12GraphicsCommandList* commandList = spriteCommon_->GetDXCommon()->GetCommandList();

    // 頂点バッファとインデックスバッファの設定
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
    commandList->IASetIndexBuffer(&indexBufferView_);

    // プリミティブのタイプを設定
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // マテリアルバッファと変換行列バッファの設定
    commandList->SetGraphicsRootConstantBufferView(0, materialBuffer_->GetGPUVirtualAddress());
    commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixBuffer_->GetGPUVirtualAddress());

    // シェーダーリソースビュー (テクスチャ)
    commandList->SetGraphicsRootDescriptorTable(2, srvHandle_);

    // 描画コマンド
    commandList->DrawIndexedInstanced(indexCount_, 1, 0, 0, 0);
}
