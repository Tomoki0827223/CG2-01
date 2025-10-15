#include "ParticleManager.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "TextureManager.h" // ★修正: TextureManagerの利用のために必要
#include "Matrix4x4.h" // Matrix4x4の利用のために必要
#include <cassert>
#include <algorithm> 
#include <cstring> // std::memcpyのために必要
#include <d3d12.h> // ID3DBlob, D3D12SerializeRootSignature のために必要
#include <dxcapi.h> // IDxcBlob のために必要

using namespace Microsoft::WRL;

// --- Matrix4x4のインライン関数がヘッダで定義されている前提 ---

ParticleManager* ParticleManager::GetInstance() {
    static ParticleManager instance;
    return &instance;
}

// 初期化処理
void ParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {
    dxCommon_ = dxCommon;
    srvManager_ = srvManager; // DirectXCommonとSRVマネージャのポインタを受け取って記録

    // ランダムエンジンの初期化 (実装省略)

    // パイプライン生成、頂点リソース生成、VBV作成などの描画リソースの初期化
    CreateRenderingResources_();
}

// グループ生成処理
void ParticleManager::CreateParticleGroup(const std::string name, const std::string textureFilePath) {
    // 登録済みの名前かチェックしてassert
    assert(particleGroups_.find(name) == particleGroups_.end());

    // 新たな空のパーティクルグループを作成し、コンテナに登録
    ParticleGroup newGroup = {};
    newGroup.textureFilePath = textureFilePath; // テクスチャファイルパスを設定

    // --- 外部モジュールを利用する処理（実装） ---

    // テクスチャを読み込む・SRVインデックスを記録
    TextureManager::GetInstance()->LoadTexture(textureFilePath);
    newGroup.textureSrvIndex = TextureManager::GetInstance()->GetSrvIndexByFilePath(textureFilePath); // SRVインデックスを記録

    // インスタンシング用リソースの生成
    ComPtr<ID3D12Resource> instancingResourceComPtr = dxCommon_->CreateBufferResource(sizeof(ParticleInstancingData) * MAX_PARTICLES);

    instancingResourceComPtr->Map(0, nullptr, &newGroup.instancingDataPtr); // ポインタを取得
    // newGroup.instancingResource は void* なので、リソースのポインタを保持する。
    // ComPtrから生ポインタを取得し、それをvoid*に代入する。（メモリ管理はComPtrに任せる）
    newGroup.instancingResource = instancingResourceComPtr.Get(); // ★修正: ComPtrのポインタをvoid*に保持 (警告を避けるためGet()を使用)

    // インスタンシング用にSRVを確保してSRVインデックスを記録
    newGroup.instancingSrvIndex = srvManager_->Allocate();

    // SRV生成 (StructuredBuffer用設定)
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = MAX_PARTICLES;
    srvDesc.Buffer.StructureByteStride = sizeof(ParticleInstancingData);
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    dxCommon_->GetDevice()->CreateShaderResourceView(
        instancingResourceComPtr.Get(), // ID3D12Resource*を取得
        &srvDesc,
        srvManager_->GetCPUDescriptorHandle(newGroup.instancingSrvIndex)
    );
    // ----------------------------------------------------

    particleGroups_[name] = newGroup;
}

// パーティクルの発生 (Emit)
void ParticleManager::Emit(const std::string name, const Vector3& position, uint32_t count) {
    // 登録済みのパーティクルグループ名かチェックしてassert
    assert(particleGroups_.count(name));
    ParticleGroup& group = particleGroups_.at(name);

    for (uint32_t i = 0; i < count; ++i) {
        // 新たなパーティクルを作成し、指定されたパーティクルグループに登録
        Particle newParticle = {};
        newParticle.position = position;
        newParticle.lifeTime = 1.0f; // 例: 寿命1秒
        newParticle.currentTime = 0.0f;
        newParticle.velocity = { 0.0f, 1.0f, 0.0f }; // 例: 上向きに移動
        newParticle.acceleration = { 0.0f, -9.8f, 0.0f }; // 例: 重力
        group.particles.push_back(newParticle);
    }
}

// 更新処理 (Update)
void ParticleManager::Update(Camera* camera, float deltaTime) {
    // ビルボード行列の計算 (実装省略)
    Matrix4x4 billboardMatrix = Matrix4x4::Identity();

    // ビュー行列とプロジェクション行列をカメラから取得
    Matrix4x4 viewMatrix = camera->GetViewMatrix();
    Matrix4x4 projectionMatrix = camera->GetProjectionMatrix();

    Matrix4x4 viewProjectionMatrix = viewMatrix * projectionMatrix;

    // 全てのパーティクルグループについて処理する
    for (auto& pair : particleGroups_) {
        ParticleGroup& group = pair.second;
        group.instancingDatas.clear();
        group.instanceCount = 0;

        for (auto it = group.particles.begin(); it != group.particles.end();) {
            Particle& p = *it;

            // 寿命に達していたらグループから外す
            if (p.currentTime >= p.lifeTime) {
                it = group.particles.erase(it);
                continue;
            }

            // 場の影響を計算 (加速)
            p.velocity = p.velocity + p.acceleration * deltaTime;

            // 移動処理 (速度を座標に加算)
            p.position = p.position + p.velocity * deltaTime;

            // 経過時間を加算
            p.currentTime += deltaTime;

            // ワールド行列を計算 (ビルボード処理を適用)
            p.worldMatrix = Matrix4x4::Scale({ 1.0f, 1.0f, 1.0f }) * billboardMatrix * Matrix4x4::Translate(p.position);

            // ワールドビュープロジェクション行列を合成
            ParticleInstancingData instanceData;
            instanceData.WVP = p.worldMatrix * viewProjectionMatrix;

            // インスタンシング用データ1個分の書き込み
            group.instancingDatas.push_back(instanceData);
            group.instanceCount++;

            ++it;
        }

        // InstancingDataをリソースに書き込む
        if (group.instanceCount > 0) {
            // インスタンシングデータをリソースポインタにコピー
            std::memcpy(group.instancingDataPtr,
                group.instancingDatas.data(),
                sizeof(ParticleInstancingData) * group.instanceCount);
        }
    }
}

// 描画処理 (Draw)
void ParticleManager::Draw() {
    auto commandList = dxCommon_->GetCommandList();

    // コマンド: ルートシグネチャを設定
    commandList->SetGraphicsRootSignature(particleRootSignature.Get());
    // コマンド: PSO (Pipeline State Object) を設定
    commandList->SetPipelineState(particlePipelineState.Get());
    // コマンド: プリミティブトポロジー (描画形状) を設定
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    // コマンド: VBV (Vertex Buffer View) を設定
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

    // 全てのパーティクルグループについて処理する
    for (auto& pair : particleGroups_) {
        ParticleGroup& group = pair.second;
        if (group.instanceCount == 0) continue;

        // DescriptorTable (Instancing DataとTexture SRV) の設定
        // RootParameter 0 (DescriptorTable) を設定

        // 連続したSRVとして、Structured Buffer (t0) と Texture (t1) を設定
        // Structured BufferのSRVインデックスがDescriptorTableの先頭になる
        D3D12_GPU_DESCRIPTOR_HANDLE tableStartHandle = srvManager_->GetGPUDescriptorHandle(group.instancingSrvIndex);

        commandList->SetGraphicsRootDescriptorTable(0, tableStartHandle);

        // コマンド: DrawCall (インスタンシング描画)
        // 頂点数4つ（クアッド）、インスタンス数は group.instanceCount
        commandList->DrawInstanced(4, group.instanceCount, 0, 0);
    }
}


// ParticleManager::CreateRenderingResources_ の実装
void ParticleManager::CreateRenderingResources_() {

    // ===================================================================
    // 1. ルートシグネチャの定義と生成 (Object3dCommonをベースにInstancing用に変更)
    // ===================================================================
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // DescriptorRange (Instancing Data, Texture SRVの2つ)
    D3D12_DESCRIPTOR_RANGE descriptorRanges[2] = {};
    // Range 0: Instancingデータ用 (t0: StructuredBuffer)
    descriptorRanges[0].BaseShaderRegister = 0;
    descriptorRanges[0].NumDescriptors = 1;
    descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    // Range 1: テクスチャ用 (t1: Texture2D)
    descriptorRanges[1].BaseShaderRegister = 1;
    descriptorRanges[1].NumDescriptors = 1;
    descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


    // RootParameter (1つ: DescriptorTableでInstancing DataとTexture SRVを渡す)
    D3D12_ROOT_PARAMETER rootParameters[1] = {};
    // 0: Instancing Data & Texture SRV用のDescriptorTable
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRanges;
    rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRanges);

    descriptionRootSignature.pParameters = rootParameters;
    // ❌ error C2039: 'NumDescriptors': 'D3D12_ROOT_SIGNATURE_DESC' のメンバーではありません
    // 👇 修正: NumParametersを使用
    descriptionRootSignature.NumParameters = _countof(rootParameters); // ★修正

    // Samplerの定義 (Object3dCommonから流用)
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
        // エラーログを出力 (Loggerクラスを仮定)
        // Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }

    hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&particleRootSignature));
    assert(SUCCEEDED(hr));


    // ===================================================================
    // 2. パイプラインステート生成
    // ===================================================================

    // 頂点インプットレイアウトの定義 (PositionとTexcoordのみ)
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

    // シェーダーのコンパイル (Particle.VS.hlsl, Particle.PS.hlsl が必要)
    ComPtr<IDxcBlob> vertexShaderBlob = dxCommon_->compileShader(L"resources/shaders/Particle.VS.hlsl", L"vs_6_0");
    ComPtr<IDxcBlob> pixelShaderBlob = dxCommon_->compileShader(L"resources/shaders/Particle.PS.hlsl", L"ps_6_0");
    assert(vertexShaderBlob != nullptr);
    assert(pixelShaderBlob != nullptr);


    // --- ブレンド設定 (加算ブレンド) ---
    D3D12_BLEND_DESC blendDesc{};
    D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc{};
    renderTargetBlendDesc.BlendEnable = TRUE;
    renderTargetBlendDesc.LogicOpEnable = FALSE;
    renderTargetBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA; // 加算ブレンド: D3D12_BLEND_SRC_ALPHA
    renderTargetBlendDesc.DestBlend = D3D12_BLEND_ONE; // 加算ブレンド: D3D12_BLEND_ONE
    renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
    renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
    renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
    renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    renderTargetBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
    renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[0] = renderTargetBlendDesc;

    // --- 深度ステンシル設定 (深度テストON, 深度書き込みOFF) ---
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // 深度書き込みを無効にする
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    // ラスタライザ設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE; // カリングなし (ビルボードのため)
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    // パイプライン生成
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPinpelineStateDesc{};
    graphicsPinpelineStateDesc.pRootSignature = particleRootSignature.Get();
    graphicsPinpelineStateDesc.InputLayout = inputLayoutDesc;
    graphicsPinpelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
    graphicsPinpelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
    graphicsPinpelineStateDesc.BlendState = blendDesc;
    graphicsPinpelineStateDesc.RasterizerState = rasterizerDesc;
    graphicsPinpelineStateDesc.DepthStencilState = depthStencilDesc;

    graphicsPinpelineStateDesc.NumRenderTargets = 1;
    graphicsPinpelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    graphicsPinpelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    graphicsPinpelineStateDesc.SampleDesc.Count = 1;
    graphicsPinpelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    graphicsPinpelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // パイプラインステートの生成
    hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPinpelineStateDesc, IID_PPV_ARGS(&particlePipelineState));
    assert(SUCCEEDED(hr));


    // ===================================================================
    // 3. 頂点バッファの作成 (ビルボード用のクアッド)
    // ===================================================================
    // ParticleManager.h で定義したインナークラス ParticleVertexData を使用
    ParticleVertexData vertices[] = {
        // Position (XYZW), Texcoord (UV)
        {{-0.5f,  0.5f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // 左上
        {{ 0.5f,  0.5f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // 右上
        {{-0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 左下
        {{ 0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // 右下
    };

    // 頂点リソースを作成
    ComPtr<ID3D12Resource> tempVertexResource = dxCommon_->CreateBufferResource(sizeof(vertices));

    // 頂点データをリソースに書き込む
    ParticleVertexData* vertexData = nullptr;
    tempVertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    std::memcpy(vertexData, vertices, sizeof(vertices));
    tempVertexResource->Unmap(0, nullptr);

    vertexResource = tempVertexResource; // メンバ変数に保持

    // VBVを作成
    vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = sizeof(vertices);
    vertexBufferView.StrideInBytes = sizeof(ParticleVertexData);
}
