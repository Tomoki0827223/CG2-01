#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include "DirectXCommon.h"
#include "D3DResourceLeakChecker.h"
#include "Input.h"
#include "externals/imgui/imgui.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Vector4.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "affine.h"


#pragma region 単位行列とTransform
// 単位行列の作成
Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; ++i) {
		result.m[i][i] = 1;
	}
	return result;
}

struct TransformVector3
{
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;

};

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
};

#pragma endregion


//MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
//	// 1.2.必要な変数の宣言とファイルを開く
//	MaterialData materialData; // 構築するMaterialData
//	std::string line; // ファイルから読んだ1行を格納するもの
//	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
//	assert(file.is_open()); // とりあえず開けなかったら止める
//	// 3.ファイルを読み、MaterialDataを構築
//	while (std::getline(file, line))
//	{
//		std::string identifier;
//		std::istringstream s(line);
//		s >> identifier;
//
//		// identifierに応じた処理
//		if (identifier == "map_Kd") {
//			std::string textureFilename;
//			s >> textureFilename;
//			// 連結してファイルパスにする
//			materialData.textureFilePath = directoryPath + "/" + textureFilename;
//		}
//	}
//	return materialData;
//}
//
//ModelData LoaObjFile(const std::string& directoryPath, const std::string& filename) {
//	
//	// 1. 中で必要となる変数の宣言
//	ModelData modelData; // 構築するModalData
//	std::vector<Vector4> positions; // 位置
//	std::vector<Vector3> normals; // 法線
//	std::vector<Vector2> texcoords; // テクスチャ座標
//	std::string line; // ファイルから読んだ1行を格納するもの
//	
//	// 2. ファイルを開く
//	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
//	assert(file.is_open()); // とりあえず開けなかったら止める
//	
//	// 3. 実際のファイルを読み込み、ModelDataを構築していく
//	while (std::getline(file, line)) {
//		std::string identifier;
//		std::istringstream s(line);
//		s >> identifier; // 先頭の識別子を読む
//		// identifierに応じた処理
//		if (identifier == "v") {
//			Vector4 position;
//			s >> position.x >> position.y >> position.z;
//			position.w = 1.0f;
//
//			position.x *= -1.0f;
//
//			positions.push_back(position);
//		}
//		else if (identifier == "vt") {
//			Vector2 texcoord;
//			s >> texcoord.x >> texcoord.y;
//
//			texcoord.y = 1.0f - texcoord.y;
//			
//			texcoords.push_back(texcoord);
//		}
//		else if (identifier == "vn") {
//			Vector3 normal;
//
//			s >> normal.x >> normal.y >> normal.z;
//
//			normal.x *= -1.0f;
//
//			normals.push_back(normal);
//		}
//		else if (identifier == "f") {
//			
//			VertexData triangle[3];
//			
//			// 面は三角形限定。その他は未対応
//			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
//				std::string vertexDefinition;
//				s >> vertexDefinition;
//
//				// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
//				std::istringstream v(vertexDefinition);
//				uint32_t elementIndeices[3];
//				for (int32_t element = 0; element < 3; ++element) {
//					std::string index;
//					std::getline(v, index, '/'); // 区切りでインデックスを読んでいく
//					elementIndeices[element] = std::stoi(index);
//				}
//
//				// 要素へのIndexから、実際の要素の値をを取得して頂点を構築する
//				Vector4 position = positions[elementIndeices[0] - 1];
//				Vector2 texcoord = texcoords[elementIndeices[1] - 1];
//				Vector3 normal = normals[elementIndeices[2] - 1];
//				VertexData vertex = { position,texcoord,normal };
//				modelData.vertices.push_back(vertex);
//				triangle[faceVertex] = { position,texcoord,normal };
//			}
//
//			// 頂点を逆順で登録することで、回り順を逆にする
//			modelData.vertices.push_back(triangle[2]);
//			modelData.vertices.push_back(triangle[1]);
//			modelData.vertices.push_back(triangle[0]);
//		}
//		else if (identifier == "mtllib") {
//			// mateialTemplateLibraryファイルの名前を取得する
//			std::string materialFilename;
//			s >> materialFilename;
//			// 基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
//			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
//		}
//	}
//	return modelData;
//
//}

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

	//ウインドウを表示する
	ShowWindow(winApp_->GetHwnd(), SW_SHOW);

#pragma endregion

#pragma region  descriptionRootSignature

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//RootParameter作成。複数設定できるので配列。今回は結果は1つだけなので長さ１の配列
	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);


	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);


	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

	if (FAILED(hr))
	{
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);

	}
	ID3D12RootSignature* rootSignature = nullptr;
	hr =  dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
#pragma endregion


#pragma region inputElementDescとgraphicsPinpelineStateDesc

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_RASTERIZER_DESC rasterizerDesc{};

	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlobComPtr = dxCommon->compileShader(L"resources/shaders/Object3D.VS.hlsl", L"vs_6_0");
	IDxcBlob* vertexShaderBlob = vertexShaderBlobComPtr.Get();
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> PicselShaderBlobComPtr = dxCommon->compileShader(L"resources/shaders/Object3D.PS.hlsl", L"ps_6_0");
	IDxcBlob* pixelShaderBlob = PicselShaderBlobComPtr.Get();
	assert(pixelShaderBlob != nullptr);


	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPinpelineStateDesc{};
	graphicsPinpelineStateDesc.pRootSignature = rootSignature;
	graphicsPinpelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPinpelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize() };
	graphicsPinpelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() };
	graphicsPinpelineStateDesc.BlendState = blendDesc;
	graphicsPinpelineStateDesc.RasterizerState = rasterizerDesc;

	graphicsPinpelineStateDesc.NumRenderTargets = 1;
	graphicsPinpelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	graphicsPinpelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	graphicsPinpelineStateDesc.SampleDesc.Count = 1;
	graphicsPinpelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	DirectX::ScratchImage mipImages = dxCommon->LoadTexture("resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	Microsoft::WRL::ComPtr<ID3D12Resource> textureResourceComPtr = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata);
	ID3D12Resource* textureResource = textureResourceComPtr.Get();


	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResourcesComPtr = dxCommon->UploadTextureData(textureResourceComPtr, mipImages);
	ID3D12Resource* intermediateResources = intermediateResourcesComPtr.Get();

	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeapComPtr = dxCommon->CreateDescriptorHeap(dxCommon->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	ID3D12DescriptorHeap* srvDescriptorHeap = srvDescriptorHeapComPtr.Get();

	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	//戦闘はImGuiが使っているのでその次を使う
	textureSrvHandleCPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//SRVの生成
	dxCommon->GetDevice()->CreateShaderResourceView(textureResource, &srvDesc, textureSrvHandleCPU);

	ID3D12PipelineState* graphicsPipelineState = nullptr;
	hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPinpelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

#pragma endregion

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourseComPtr = dxCommon->CreateBufferResource(sizeof(VertexData) * 3);
	ID3D12Resource* vertexResourse = vertexResourseComPtr.Get();

	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResourseComPtr = dxCommon->CreateBufferResource(sizeof(Matrix4x4));
	ID3D12Resource* wvpResourse = wvpResourseComPtr.Get();

	//ここから02_01確認課題
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourseComPtr = dxCommon->CreateBufferResource(sizeof(VertexData));
	ID3D12Resource* materialResourse = materialResourseComPtr.Get();
	Vector4* materialData = nullptr;
	materialResourse->Map(0, nullptr, reinterpret_cast<void**>(&materialData));


	//こここで色かえられるよ
	*materialData = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//こここで色かえられるよ

	//ここまで
	Matrix4x4* wvpData = nullptr;
	wvpResourse->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	*wvpData = MakeIdentity4x4();

	//頂点バッファービューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferview{};

	vertexBufferview.BufferLocation = vertexResourse->GetGPUVirtualAddress();
	vertexBufferview.SizeInBytes = sizeof(VertexData) * 3;
	vertexBufferview.StrideInBytes = sizeof(VertexData);

	//頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	//書き込むためのアドレスを取得
	vertexResourse->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	//左下
	vertexData[0].position = { -0.5, -0.5f, 0.0f, 1.0f };
	vertexData[0].texcoord = { 0.0f, 1.0f };
	//上
	vertexData[1].position = { 0.0f, 0.5f, 0.0f, 1.0f };
	vertexData[1].texcoord = { 0.5f,0.0f };
	//右下
	vertexData[2].position = { 0.5f, -0.5f, 0.0f, 1.0f };
	vertexData[2].texcoord = { 1.0f, 1.0f };
	
	D3D12_VIEWPORT viewport{};

	viewport.Width = winApp_->kClientWidth;
	viewport.Height = winApp_->kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D12_RECT scissorRect{};

	scissorRect.left = 0;
	scissorRect.right = winApp_->kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = winApp_->kClientHeight;


	TransformVector3 transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	TransformVector3 cameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-5.0f} };


	MSG msg{};

	//ImGuiの初期化
	dxCommon->InitializeImGui();

	ImGui::StyleColorsDark();

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

            // ゲーム処理
			transform.rotate.y += 0.03f;
			Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
			Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			Matrix4x4 viewMatrix = Inverse(cameraMatrix);
			Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(winApp_->kClientWidth) / float(winApp_->kClientHeight), 0.1f, 100.0f);
			Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
			//	*transformationMatrixData = 
			*wvpData = worldViewProjectionMatrix;


            // 描画前処理
            dxCommon->PreDraw();


			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGui::Begin("SetColor");
			ImGui::ColorEdit4("*materialData", &materialData->x);
			ImGui::End();
			ImGui::ShowDemoWindow();
			ImGui::Render();

			//描画用のDescriptorHeapの設定
			ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap };
			dxCommon->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);

			dxCommon->GetCommandList()->RSSetViewports(1, &viewport);
			dxCommon->GetCommandList()->RSSetScissorRects(1, &scissorRect);
			//dxCommon->GetCommandList()->RSSetViewports(1, &dxCommon->InitializeViewportAndScissorRect());
			//dxCommon->GetCommandList()->RSSetScissorRects(1, &dxCommon->InitializeScissorRect());
			
			//RootSignatureを設定。PSOに設定しているけど別途設定が必要
			dxCommon->GetCommandList()->SetGraphicsRootSignature(rootSignature);
			dxCommon->GetCommandList()->SetPipelineState(graphicsPipelineState);
			dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferview);//
			dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourse->GetGPUVirtualAddress());
			dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResourse->GetGPUVirtualAddress());
			dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
			//形状を設定。PSOに設定しているものとはまた別、同じものを設定すると考えておけば良い
			dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			//描画　（DrawCall/drawコール）　。　3頂点で1つのインスタンス。
			dxCommon->GetCommandList()->DrawInstanced(3, 1, 0, 0);


            // 描画後処理
            dxCommon->PostDraw();
        }
    }


	//Windows終了
	winApp_->Finalize();

	delete input;
	delete winApp_;
	delete dxCommon;
	//winApp_ = nullptr;


	//CloseHandle(fenceEvent);

	//CoUninitialize();
	return 0;
}