#include "ParticleManager.h"
// ↓ .hから移動したインクルードと、追加で必要なインクルード
#include "TextureManager.h"
#include "SrvManager.h"
#include "Camera.h"
#include <cassert>
#include <algorithm>
#include <string> 
#include <dxcapi.h> // CompileShaderの実装に必要

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
    constantBuffer = CreateBufferResource(device, sizeof(ConstBufferData));
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


    // --- 2. PipelineStateObjectの作成 ---

    // InputLayout (頂点データとインスタンスデータ)
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        // Vertex Data (スロット0)
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        // Instance Data (スロット1)
        {"INSTANCE_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
        {"INSTANCE_VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
        {"INSTANCE_COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
        {"INSTANCE_START_SCALE", 0, DXGI_FORMAT_R32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
        {"INSTANCE_END_SCALE", 0, DXGI_FORMAT_R32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
        {"INSTANCE_LIFETIME", 0, DXGI_FORMAT_R32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
        {"INSTANCE_CURRENT_TIME", 0, DXGI_FORMAT_R32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
        {"INSTANCE_IS_ACTIVE", 0, DXGI_FORMAT_R32_SINT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
    };

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);

    // シェーダーのコンパイルと読み込み
    // ↓ 修正: IDxcBlob -> ID3DBlob に変更
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;

    // compileShader が ID3DBlob を返すようになったため、代入可能になる
    vertexShaderBlob = dxCommon_->compileShader(L"resources/shaders/Particle.VS.hlsl", L"vs_6_0");
    pixelShaderBlob = dxCommon_->compileShader(L"resources/shaders/Particle.PS.hlsl", L"ps_6_0");
    assert(vertexShaderBlob && pixelShaderBlob);

    // PipelineStateの作成
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = rootSignature.Get();

    // ID3DBlob のメソッドを使用 (そのまま)
    psoDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
    psoDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };


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
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
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
    // 💡 修正: LoadTextureはvoidを返すため、Loadを呼ぶ
    TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");

    // 💡 修正: ハンドルはファイルパスから取得し、indexを保持する (テクスチャ番号取得)
    textureHandle = TextureManager::GetInstance()->GetSrvIndexByFilePath("resources/uvChecker.png");
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
            particles[i].currentTime += deltaTime;

            // 💡 修正: Vector3 * float の演算子がないため、メンバごとに計算
            particles[i].position.x += particles[i].velocity.x * deltaTime;
            particles[i].position.y += particles[i].velocity.y * deltaTime;
            particles[i].position.z += particles[i].velocity.z * deltaTime;

            if (particles[i].currentTime >= particles[i].lifetime) {
                particles[i].isActive = 0;
            }

            instancingMap[i] = particles[i];
        }
        else {
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
    // 💡 修正: Camera.cppで実装した関数を呼び出す
    cData.cameraRight = camera_->GetWorldRightVector();
    cData.cameraUp = camera_->GetWorldUpVector();

    // 定数バッファに書き込み
    void* mapData = nullptr;
    constantBuffer->Map(0, nullptr, &mapData);
    memcpy(mapData, &cData, sizeof(ConstBufferData));
    constantBuffer->Unmap(0, nullptr);

    // CBVの設定 (ルートパラメータ0)
    commandList->SetGraphicsRootConstantBufferView(0, constantBuffer->GetGPUVirtualAddress());

    // SRVの設定 (ルートパラメータ1)
    // 💡 修正: GetSrvHandleGPUByFilePathを使用
    D3D12_GPU_DESCRIPTOR_HANDLE textureHandleGPU = TextureManager::GetInstance()->GetSrvHandleGPUByFilePath("resources/uvChecker.png");
    commandList->SetGraphicsRootDescriptorTable(1, textureHandleGPU);


    // --- 描画実行 (DrawInstanced) ---
    UINT activeCount = 0;
    for (int i = 0; i < kMaxParticles; ++i) {
        if (particles[i].isActive) {
            activeCount++;
        }
    }
    commandList->DrawInstanced(4, activeCount, 0, 0);
}