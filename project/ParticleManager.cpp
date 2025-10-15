#include "ParticleManager.h"
// ParticleManager.h から移動したインクルード
#include "TextureManager.h"
#include "SrvManager.h"
#include "Camera.h"
#include <cassert>
#include <algorithm>
#include <string> // CompileShaderで使用される可能性のため追加

#pragma region ヘルパー関数

// リソース作成用のヘルパー関数 (DirectXCommonから持ってくるか、新しく作る必要がありますが、ここでは簡略化して実装します)
// 実際にはDirectXCommon::CreateBufferResourceなどを使うべきです
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
    HRESULT hr;
    Microsoft::WRL::ComPtr<ID3D12Resource> resource;

    D3D12_HEAP_PROPERTIES heapProp{};
    heapProp.Type = D3D12_HEAP_TYPE_UPLOAD; // CPUからGPUへの転送に使う

    D3D12_RESOURCE_DESC resDesc{};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = sizeInBytes;
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    hr = device->CreateCommittedResource(
        &heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&resource));
    assert(SUCCEEDED(hr));
    return resource;
}

// ConstantBuffer構造体 (シェーダー側のcbuffer cb0と対応)
struct ConstBufferData {
    Matrix4x4 viewProjection;
    Vector3 cameraRight;
    float pad1; // 16バイト境界のためのパディング
    Vector3 cameraUp;
    float pad2; // 16バイト境界のためのパディング
};

#pragma endregion

void ParticleManager::Initialize(DirectXCommon* dxCommon, Camera* camera) {
    assert(dxCommon);
    assert(camera);

    dxCommon_ = dxCommon;
    camera_ = camera;

    // 1. リソース作成 (頂点バッファ, インスタンシングバッファ, 定数バッファ)
    CreateResources();

    // 2. パイプライン作成 (ルートシグネチャ, パイプラインステート)
    CreatePipeline();

    // 3. テクスチャロード
    LoadTexture();

    // パーティクル配列の初期化
    std::fill(particles, particles + kMaxParticles, ParticleData{});
}

void ParticleManager::CreateResources() {
    ID3D12Device* device = dxCommon_->GetDevice();

    // --- 1. ビルボード用頂点リソースの作成 (4頂点のクアッド) ---
    ParticleVertex vertices[4] = {
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}}, // 左下
        {{-0.5f, 0.5f, 0.0f},  {0.0f, 0.0f}}, // 左上
        {{0.5f, -0.5f, 0.0f},  {1.0f, 1.0f}}, // 右下
        {{0.5f, 0.5f, 0.0f},   {1.0f, 0.0f}}  // 右上
    };
    const UINT sizeVB = sizeof(vertices);
    vertexBuffer = CreateBufferResource(device, sizeVB);

    // 頂点データをマップしてコピー
    ParticleVertex* vertexMap = nullptr;
    vertexBuffer->Map(0, nullptr, (void**)&vertexMap);
    memcpy(vertexMap, vertices, sizeVB);
    vertexBuffer->Unmap(0, nullptr);

    // VB Viewの設定
    vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vbView.SizeInBytes = sizeVB;
    vbView.StrideInBytes = sizeof(ParticleVertex);


    // --- 2. インスタンスデータ用リソースの作成 (ParticleData * kMaxParticles) ---
    const UINT sizeInstancing = sizeof(ParticleData) * kMaxParticles;
    instancingBuffer = CreateBufferResource(device, sizeInstancing);

    // インスタンスデータをマップ (Updateで使うために保持)
    instancingBuffer->Map(0, nullptr, (void**)&instancingMap);

    // 初期データをコピー (全て非アクティブで初期化)
    std::fill(instancingMap, instancingMap + kMaxParticles, ParticleData{});

    // --- 3. 定数バッファの作成 (ConstBufferData: Camera情報用) ---
    // 💡 修正: constantBuffer のスペルミス (ComPtr) はヘッダーで修正済み。ここでは正常に動作する。
    constantBuffer = CreateBufferResource(device, sizeof(ConstBufferData));

    // 定数バッファはDrawでGPUへ転送するため、マップしたままにする必要はありません。
}

void ParticleManager::CreatePipeline() {
    ID3D12Device* device = dxCommon_->GetDevice();
    HRESULT hr;

    // --- 1. RootSignatureの作成 ---

    // Root Parameter (CBV, SRVの2つ)
    D3D12_ROOT_PARAMETER rootParameters[2]{};

    // 0: 定数バッファ (b0) -> カメラ情報
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].Descriptor.ShaderRegister = 0; // b0

    // 1: SRV (t0) -> テクスチャ
    D3D12_DESCRIPTOR_RANGE descriptorRange{};
    descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange.NumDescriptors = 1; // SRVを1つ
    descriptorRange.BaseShaderRegister = 0; // t0
    descriptorRange.RegisterSpace = 0;
    descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // ピクセルシェーダでのみ使用
    rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRange;
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;

    // Samplerの設定
    D3D12_STATIC_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.NumParameters = _countof(rootParameters);
    rootSignatureDesc.pStaticSamplers = &samplerDesc;
    rootSignatureDesc.NumStaticSamplers = 1;

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        assert(false);
    }

    hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    assert(SUCCEEDED(hr));

    // シェーダーのコンパイルと読み込み
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;
    // 💡 修正: DirectXCommonに CompileShader がないというエラーに対応するため、
    // ユーザーに DirectXCommon.h の修正を促します。ここでは呼び出しは維持。
    vertexShaderBlob = dxCommon_->CompileShader(L"resources/shaders/Particle.VS.hlsl", L"vs_6_0");
    pixelShaderBlob = dxCommon_->CompileShader(L"resources/shaders/Particle.PS.hlsl", L"ps_6_0");
    assert(vertexShaderBlob && pixelShaderBlob);

    // --- 2. PipelineStateObjectの作成 ---

    // InputLayout (頂点データとインスタンスデータ)
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[_countof(ParticleData) + 2]{};

    // Vertex Data (4頂点のクアッド用)
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

    // Instance Data (ParticleData)
    // ParticleManager.hのstruct ParticleDataのメンバ順に合わせる
    int instanceElementIndex = 2;
    // position
    inputElementDescs[instanceElementIndex].SemanticName = "INSTANCE_POSITION";
    inputElementDescs[instanceElementIndex].SemanticIndex = 0;
    inputElementDescs[instanceElementIndex].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[instanceElementIndex].InputSlot = 1; // インスタンスデータはスロット1
    inputElementDescs[instanceElementIndex].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
    inputElementDescs[instanceElementIndex].InstanceDataStepRate = 1;
    instanceElementIndex++;

    // velocity (VSでは使用しないが、構造体合わせとセマンティクスが必要)
    inputElementDescs[instanceElementIndex].SemanticName = "INSTANCE_VELOCITY";
    inputElementDescs[instanceElementIndex].SemanticIndex = 0;
    inputElementDescs[instanceElementIndex].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[instanceElementIndex].InputSlot = 1;
    inputElementDescs[instanceElementIndex].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[instanceElementIndex].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
    inputElementDescs[instanceElementIndex].InstanceDataStepRate = 1;
    instanceElementIndex++;

    // color
    inputElementDescs[instanceElementIndex].SemanticName = "INSTANCE_COLOR";
    inputElementDescs[instanceElementIndex].SemanticIndex = 0;
    inputElementDescs[instanceElementIndex].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[instanceElementIndex].InputSlot = 1;
    inputElementDescs[instanceElementIndex].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[instanceElementIndex].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
    inputElementDescs[instanceElementIndex].InstanceDataStepRate = 1;
    instanceElementIndex++;

    // startScale
    inputElementDescs[instanceElementIndex].SemanticName = "INSTANCE_START_SCALE";
    inputElementDescs[instanceElementIndex].SemanticIndex = 0;
    inputElementDescs[instanceElementIndex].Format = DXGI_FORMAT_R32_FLOAT;
    inputElementDescs[instanceElementIndex].InputSlot = 1;
    inputElementDescs[instanceElementIndex].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[instanceElementIndex].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
    inputElementDescs[instanceElementIndex].InstanceDataStepRate = 1;
    instanceElementIndex++;

    // endScale
    inputElementDescs[instanceElementIndex].SemanticName = "INSTANCE_END_SCALE";
    inputElementDescs[instanceElementIndex].SemanticIndex = 0;
    inputElementDescs[instanceElementIndex].Format = DXGI_FORMAT_R32_FLOAT;
    inputElementDescs[instanceElementIndex].InputSlot = 1;
    inputElementDescs[instanceElementIndex].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[instanceElementIndex].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
    inputElementDescs[instanceElementIndex].InstanceDataStepRate = 1;
    instanceElementIndex++;

    // lifetime
    inputElementDescs[instanceElementIndex].SemanticName = "INSTANCE_LIFETIME";
    inputElementDescs[instanceElementIndex].SemanticIndex = 0;
    inputElementDescs[instanceElementIndex].Format = DXGI_FORMAT_R32_FLOAT;
    inputElementDescs[instanceElementIndex].InputSlot = 1;
    inputElementDescs[instanceElementIndex].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[instanceElementIndex].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
    inputElementDescs[instanceElementIndex].InstanceDataStepRate = 1;
    instanceElementIndex++;

    // currentTime
    inputElementDescs[instanceElementIndex].SemanticName = "INSTANCE_CURRENT_TIME";
    inputElementDescs[instanceElementIndex].SemanticIndex = 0;
    inputElementDescs[instanceElementIndex].Format = DXGI_FORMAT_R32_FLOAT;
    inputElementDescs[instanceElementIndex].InputSlot = 1;
    inputElementDescs[instanceElementIndex].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[instanceElementIndex].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
    inputElementDescs[instanceElementIndex].InstanceDataStepRate = 1;
    instanceElementIndex++;

    // isActive
    inputElementDescs[instanceElementIndex].SemanticName = "INSTANCE_IS_ACTIVE";
    inputElementDescs[instanceElementIndex].SemanticIndex = 0;
    inputElementDescs[instanceElementIndex].Format = DXGI_FORMAT_R32_SINT; // intはSINT
    inputElementDescs[instanceElementIndex].InputSlot = 1;
    inputElementDescs[instanceElementIndex].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[instanceElementIndex].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
    inputElementDescs[instanceElementIndex].InstanceDataStepRate = 1;
    // instanceElementIndex++; // ここで終わり

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(ParticleData) + 2;


    // シェーダーのコンパイルと読み込み
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;
    vertexShaderBlob = dxCommon_->CompileShader(L"resources/shaders/Particle.VS.hlsl", L"vs_6_0");
    pixelShaderBlob = dxCommon_->CompileShader(L"resources/shaders/Particle.PS.hlsl", L"ps_6_0");
    assert(vertexShaderBlob && pixelShaderBlob);

    // BlendState (加算合成: Additive Blending)
    D3D12_BLEND_DESC blendDesc{};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    // レンダーターゲット0 の設定
    D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc{};
    renderTargetBlendDesc.BlendEnable = TRUE;
    renderTargetBlendDesc.LogicOpEnable = FALSE;
    // 加算ブレンド: DestColor = SrcColor * SrcAlpha + DestColor * 1.0
    renderTargetBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
    renderTargetBlendDesc.DestBlend = D3D12_BLEND_ONE;
    renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
    // アルファ値は無視（通常1.0）
    renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
    renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
    renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    blendDesc.RenderTarget[0] = renderTargetBlendDesc;

    // RasterizerState (通常通り)
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK; // ビルボードはカリングされないはずだが、一応設定
    rasterizerDesc.DepthClipEnable = TRUE;

    // DepthStencilState (デプス参照は行うが、書き込みはしない)
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // ここが重要: 深度書き込みを無効化
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;


    // PipelineStateの作成
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
    psoDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
    psoDesc.BlendState = blendDesc;
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.InputLayout = inputLayoutDesc;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));
    assert(SUCCEEDED(hr));
}

void ParticleManager::LoadTexture() {
    // 既存のTextureManagerとSrvManagerを使ってテクスチャをロード
    // 例として「resources/uvChecker.png」または専用のパーティクルテクスチャを使用
    textureHandle = TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");
    // または、専用のパーティクルテクスチャ (例: "resources/spark.png") を用意してロード
}

void ParticleManager::Emit(const Vector3& position, const Vector3& velocity,
    const Vector4& color, float startScale, float endScale, float lifetime) {
    // 非アクティブなパーティクルを探す
    for (int i = 0; i < kMaxParticles; ++i) {
        if (!particles[i].isActive) {
            particles[i].position = position;
            particles[i].velocity = velocity;
            particles[i].color = color;
            particles[i].startScale = startScale;
            particles[i].endScale = endScale;
            particles[i].lifetime = lifetime;
            particles[i].currentTime = 0.0f;
            particles[i].isActive = 1;
            return; // 1つ見つけたら終了
        }
    }
}

void ParticleManager::Update(float deltaTime) {
    for (int i = 0; i < kMaxParticles; ++i) {
        if (particles[i].isActive) {
            // 経過時間を更新
            particles[i].currentTime += deltaTime;

            // 💡 修正: Vector3 * float の演算子オーバーロードがないため、メンバごとに計算する
            // particles[i].position = particles[i].position + particles[i].velocity * deltaTime;
            particles[i].position.x += particles[i].velocity.x * deltaTime;
            particles[i].position.y += particles[i].velocity.y * deltaTime;
            particles[i].position.z += particles[i].velocity.z * deltaTime;

            // 寿命チェック
            if (particles[i].currentTime >= particles[i].lifetime) {
                particles[i].isActive = 0; // 終了
            }

            // GPUに転送するためのバッファにデータをコピー
            instancingMap[i] = particles[i];
        }
        else {
            // 非アクティブなパーティクルも、GPUへの転送バッファを更新
            instancingMap[i] = particles[i];
        }
    }
}

void ParticleManager::Draw(ID3D12GraphicsCommandList* commandList) {
    // コマンドリストの設定
    commandList->SetGraphicsRootSignature(rootSignature.Get());
    commandList->SetPipelineState(pipelineState.Get());

    // プリミティブ形状の設定
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP); // 4頂点で描画するためTRISTRIP

    // VB/IBの設定 
    commandList->IASetVertexBuffers(0, 1, &vbView);
    // インスタンスデータバッファの設定 (スロット1)
    D3D12_VERTEX_BUFFER_VIEW instancingVBView{};
    instancingVBView.BufferLocation = instancingBuffer->GetGPUVirtualAddress();
    instancingVBView.SizeInBytes = sizeof(ParticleData) * kMaxParticles;
    instancingVBView.StrideInBytes = sizeof(ParticleData);
    commandList->IASetVertexBuffers(1, 1, &instancingVBView);


    // --- 定数バッファの更新と設定 ---
    ConstBufferData cData{};
    cData.viewProjection = camera_->GetViewProjectionMatrix();

    // 💡 修正: Camera::GetWorld*Vector がないというエラーに対応するため、
    // ユーザーに Camera.h の修正を促します。ここでは呼び出しは維持。
    cData.cameraRight = camera_->GetWorldRightVector();
    cData.cameraUp = camera_->GetWorldUpVector();

    // 定数バッファに書き込み
    void* mapData = nullptr;
    // 💡 修正: constantBuffer のスペルミス (ComPtr) はヘッダーで修正済み。ここでは正常に動作する。
    constantBuffer->Map(0, nullptr, &mapData);
    memcpy(mapData, &cData, sizeof(ConstBufferData));
    constantBuffer->Unmap(0, nullptr);

    // CBVの設定 (ルートパラメータ0)
    commandList->SetGraphicsRootConstantBufferView(0, constantBuffer->GetGPUVirtualAddress());

    // SRVの設定 (ルートパラメータ1)
    D3D12_GPU_DESCRIPTOR_HANDLE textureHandleGPU = TextureManager::GetInstance()->GetSrvHandleGPU(textureHandle);
    commandList->SetGraphicsRootDescriptorTable(1, textureHandleGPU);


    // --- 描画実行 (DrawInstanced) ---
    UINT activeCount = 0;
    for (int i = 0; i < kMaxParticles; ++i) {
        if (particles[i].isActive) {
            activeCount++;
        }
    }

    // DrawInstanced(頂点数, インスタンス数, 頂点オフセット, インスタンスオフセット)
    commandList->DrawInstanced(4, activeCount, 0, 0);

    // 備考: 4頂点で1枚のビルボードを描画するため、プリミティブトポロジはD3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIPが適切
}