#include "Sprite.h"
#include "SpriteCommon.h"

Sprite::~Sprite()
{
    delete sprite;
}

void Sprite::Initialize(SpriteCommon* spriteCommon)
{
	this->spriteCommon_ = spriteCommon;
}

void Sprite::Update()
{
	//ここから03_01
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(depthStencilResouce.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	//ここから03_01

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPinpelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	//Material用のResourceを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = CreateBufferResource(device, sizeof(Material));
	Material* materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	////こここで色かえられるよ
	materialData->color = { 1.0f,1.0f,1.0f,1.0f };
	materialData->endleLighting = true;
	materialData->uvTransform = MakeIdentity4x4();

	const uint32_t kNumInstance = 10;
	//Material用のResourceを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource = CreateBufferResource(device, sizeof(TransformationMatrix) * kNumInstance);
	TransformationMatrix* instancingData = nullptr;
	instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&instancingData));
	////こここで色かえられるよ
	for (uint32_t index = 0; index < kNumInstance; index++)
	{
		instancingData[index].WVP = MakeIdentity4x4();
		instancingData[index].world = MakeIdentity4x4();
	}

	TransformVector3 transforms[kNumInstance];
	for (uint32_t index = 0; index < kNumInstance; index++)
	{
		transforms[index].scale = { 1.0f,1.0f,1.0f };
		transforms[index].rotate = { 0.0f,0.0f,0.0f };
		transforms[index].translate = { index * 0.1f,index * 0.1f,index * 0.1f };
	}


	bool useMonsterBall = false;
	TransformVector3 transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	//Resourcef
	//const uint32_t kSubdivision = 36;

	//VertexResourceを生成
	//ID3D12Resource* vertexResource = CreateBufferResource(device, sizeof(VertexData) * kSubdivision * kSubdivision * 6);

	//モデル読み込み
	//ModelData modelData = LoaObjFile("resources", "Bunny.obj");
	//ModelData modelData = LoaObjFile("resources", "plane.obj");

	//頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = CreateBufferResource(device, sizeof(VertexData) * modelData.vertices.size());
	//頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();// リソースの先頭のアドレスから使う
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());// 使用するリソースのサイズは頂点のサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);// 1頂点あたりのサイズ

	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));// 書き込むためのアドレスを取得
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());// 頂点データをリソースにコピー

	//DepthStencilTextureを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = CreateDepthStencilTextureResource(device, kClientwidth, kClientHeight);

	//VertexBufferResourceを生成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = CreateBufferResource(device, sizeof(VertexData) * 6);

	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = CreateBufferResource(device, sizeof(uint32_t) * 6);

	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};

	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;

	uint32_t* indexDataSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0;
	indexDataSprite[1] = 1;
	indexDataSprite[2] = 2;
	indexDataSprite[3] = 1;
	indexDataSprite[4] = 3;
	indexDataSprite[5] = 2;

	//Sprite用のマテリアルリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = CreateBufferResource(device, sizeof(Material));
	Material* materialDataSprite = nullptr;
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	materialDataSprite->color = { 1.0f,1.0f,1.0f,1.0f };
	materialDataSprite->endleLighting = false;
	materialDataSprite->uvTransform = MakeIdentity4x4();

	//ライティング
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResorce = CreateBufferResource(device, sizeof(DirectionaLight));
	DirectionaLight* directionalLightData = nullptr;
	directionalLightResorce->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightData->intensity = 1.0f;

	//TransformationMatrixResource
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = CreateBufferResource(device, sizeof(TransformationMatrix));
	TransformationMatrix* transformationMatrixDataSprite = nullptr;
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	transformationMatrixDataSprite->WVP = MakeIdentity4x4();
	transformationMatrixDataSprite->world = MakeIdentity4x4();

	// Sprite用のTransfomationMatrix用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = CreateBufferResource(device, sizeof(TransformationMatrix));
	// データを書き込む
	TransformationMatrix* wvpDeta = nullptr;
	// 書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpDeta));
	// 単位行列を書き込んでおく
	wvpDeta->world = MakeIdentity4x4();
	wvpDeta->WVP = MakeIdentity4x4();

	////vetexResourceSprite頂点バッファーを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{ };
	//リソースの先頭のアドレスから使う
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
	//1頂点当たりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);
	//頂点リソースにデータを書き込む
	VertexData* vertexDataSprite = nullptr;
	//書き込むためのアドレスを取得
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));

	// 1枚目の三角形
	vertexDataSprite[0].position = { 0.0f, 360.0f, 0.0f, 1.0f };// 左下
	vertexDataSprite[0].texcoord = { 0.0f, 1.0f };
	vertexDataSprite[0].nomal = { 0.0f, 0.0f, -1.0f };
	vertexDataSprite[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };// 左上
	vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	vertexDataSprite[1].nomal = { 0.0f, 0.0f, -1.0f };
	vertexDataSprite[2].position = { 640.0f, 360.0f, 0.0f, 1.0f };// 右下
	vertexDataSprite[2].texcoord = { 1.0f, 1.0f };
	vertexDataSprite[2].nomal = { 0.0f, 0.0f, -1.0f };
	// 2枚目の三角形
	vertexDataSprite[3].position = { 640.0f, 0.0f, 0.0f, 1.0f };// 右上
	vertexDataSprite[3].texcoord = { 1.0f, 0.0f };
	vertexDataSprite[3].nomal = { 0.0f, 0.0f, -1.0f };

	D3D12_VIEWPORT viewport{};

	viewport.Width = kClientwidth;
	viewport.Height = kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D12_RECT scissorRect{};

	scissorRect.left = 0;
	scissorRect.right = kClientwidth;
	scissorRect.top = 0;
	scissorRect.bottom = kClientHeight;


	TransformVector3 transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	TransformVector3 cameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	const uint32_t descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	GetCPUDescriptorHandle(rtvDescriptorHeap, descriptorSizeRTV, 0);


	DirectX::ScratchImage mipimage2 = LoadTexture("resources/monsterBall.png");
	const DirectX::TexMetadata& metadata2 = mipimage2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = CreateTextureResource(device, metadata2);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResources2 = UploadTextureData(textureResource2, mipimage2, device, commandList);

	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = kNumInstance;
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(TransformationMatrix);

	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 2);
	//SRVの生成
	device->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);

	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU = GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 3);
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU = GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 3);
	device->CreateShaderResourceView(instancingResource.Get(), &instancingSrvDesc, instancingSrvHandleCPU);


	//DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
	DirectX::ScratchImage mipImages2 = LoadTexture(modelData.material.textureFilePath);
	const DirectX::TexMetadata& metadata = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = CreateTextureResource(device, metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResources = UploadTextureData(textureResource, mipImages2, device, commandList);
	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	//戦闘はImGuiが使っているのでその次を使う
	textureSrvHandleCPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//SRVの生成
	device->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);

	MSG msg{};
}

void Sprite::Draw()
{
	// 描画先のRTVとDSVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap.Get() };
	commandList->SetDescriptorHeaps(1, descriptorHeaps);

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	//RootSignatureを設定。PSOに設定しているけど別途設定が必要
	commandList->SetGraphicsRootSignature(rootSignature.Get());
	commandList->SetPipelineState(graphicsPipelineState.Get());

	//Sphere
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	//形状を設定。PSOに設定しているものとはまた別、同じものを設定すると考えておけば良い
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	//wvp用のCBufferの場所を設定
	//commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResorce->GetGPUVirtualAddress());

	// 他の設定諸々
	// instancing用のDataを読むためにStructured Buffer SRVを設定する
	commandList->SetGraphicsRootDescriptorTable(1, instancingSrvHandleGPU);
	// 他の設定諸々
	//描画! 6頂点の板ポリゴンを、kNumInstance(今回は10)だけInstance描画を行う
	commandList->DrawInstanced(UINT(modelData.vertices.size()), kNumInstance, 0, 0);
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
