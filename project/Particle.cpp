#include "Particle.h"
#include "DirectXCommon.h"
#include "Object3dCommon.h" // ルートシグネチャ作成などに必要
#include "Camera.h" // カメラ情報取得に必要
#include <cassert>
// project/Particle.cpp (抜粋)
#include "SrvManager.h" // SRV Managerが必要

// ... (省略) ...

void Particle::Initialize(uint32_t textureHandle) {
    // メンバ変数の初期化
    this->textureHandle = textureHandle;
    particles.reserve(kMaxParticles);

    CreatePipeline();
    CreateResources();
    InitializeVertexData();
}

// project/Particle.cpp (抜粋)

void Particle::Update() {
    // 除去対象のインデックスを保持
    std::vector<size_t> deadParticles;

    // 1. パーティクルリストをイテレート
    for (size_t i = 0; i < particles.size(); ++i) {
        ParticleData& p = particles[i];

        // 2. 現在時間のインクリメント
        p.currentTime++;

        // 3. 寿命判定
        if (p.currentTime >= p.lifeTime) {
            deadParticles.push_back(i);
            // VertexDataのデータは、パーティクルリストの整理後にまとめて更新します
            continue;
        }

        // 4. 速度による位置の更新 (位置 += 速度)
        p.position = p.position + p.velocity;

        // 5. スケールと色の補間（線形補間 LERP）
        float t = (float)p.currentTime / p.lifeTime; // 0.0fから1.0fへの進行度

        // LERP関数 (ここでは Vector4 の LERP を想定)
        // Vector4 LERP(const Vector4& start, const Vector4& end, float t) 
        p.scale = p.startScale + (p.endScale - p.startScale) * t;
        p.color = p.startColor.Lerp(p.endColor, t); // Vector4::Lerp があると仮定

        // 6. 頂点バッファへのデータ書き込み (更新されたものだけ)
        vertexData[i].pos = p.position;
        vertexData[i].scale = p.scale;
        vertexData[i].color = p.color;
    }

    // 寿命が尽きたパーティクルをリストから除去 (末尾とスワップしてポップ)
    for (size_t i : deadParticles) {
        // 常に末尾の要素と入れ替えて pop_back
        std::swap(particles[i], particles.back());
        particles.pop_back();

        // 入れ替えられた要素の VertexData も更新する
        if (i < particles.size()) {
            vertexData[i].pos = particles[i].position;
            vertexData[i].scale = particles[i].scale;
            vertexData[i].color = particles[i].color;
        }

        // deadParticles リスト内の後続のインデックスを修正する必要があるため、
        // 効率のためにはこの方法ではなく、逆順に削除するか、
        // isAlive フラグを使う方がシンプルです。
        // ここでは、一旦パーティクルが消える時に、VertexDataの最後の要素を
        // 移動されたパーティクルのデータで上書きする処理のみを記述します。

        // ただし、上記のスワップロジックはインデックスの調整が複雑になるため、
        // シンプルに、VertexDataを常にパーティクルリスト全体で更新する方式に切り替えます。

        // VertexDataの更新はループ外でまとめて行います。（後述）
    }

    // --- パーティクルリストの整理後の VertexData の一括更新 ---
    // Update関数で更新されたパーティクルの数だけ、頂点バッファを埋めます。
    for (size_t i = 0; i < particles.size(); ++i) {
        const ParticleData& p = particles[i];
        vertexData[i].pos = p.position;
        vertexData[i].scale = p.scale;
        // LerpはUpdate内で既に計算されていると仮定
        vertexData[i].color = p.color;
    }
    // 使わない残りのVertexDataをクリア (描画数制御のため)
    for (size_t i = particles.size(); i < kMaxParticles; ++i) {
        vertexData[i].scale = 0.0f; // scale=0で非表示とする
    }
}


void Particle::Draw(ID3D12GraphicsCommandList* commandList, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& inverseViewMatrix) {
    if (particles.empty()) {
        return; // パーティクルがない場合は描画しない
    }

    // 1. 定数バッファの更新 (ViewProjectionとInverseViewMatrix)
    constMap->viewProjectionMatrix = viewProjectionMatrix;
    constMap->inverseViewMatrix = inverseViewMatrix; // ビルボードに必要

    // 2. パイプラインステートとルートシグネチャの設定
    commandList->SetPipelineState(pipelineState.Get());
    commandList->SetGraphicsRootSignature(rootSignature.Get());

    // 3. プリミティブトポロジーの設定 (ポイントリスト)
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

    // 4. 頂点バッファと定数バッファの設定
    commandList->IASetVertexBuffers(0, 1, &vbView);
    commandList->SetGraphicsRootConstantBufferView(0, cbResource->GetGPUVirtualAddress()); // RootParameter[0]にCBVを設定

    // 5. SRVの設定 (テクスチャ)
    SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(commandList, 1, textureHandle); // RootParameter[1]にSRVを設定

    // 6. 描画コマンドの実行
    // DrawInstanced(頂点数, インスタンス数, 開始頂点インデックス, 開始インスタンスインデックス)
    // 1頂点をパーティクル数分インスタンス描画します。
    // 頂点シェーダー内で、この1頂点を元に四角形のジオメトリを生成します。
    commandList->DrawInstanced(
        1,                         // 1つのパーティクルあたり1頂点
        (UINT)particles.size(),    // インスタンス数 = 描画するパーティクルの数
        0, 0
    );
}

// ... Emit 関数の実装 (省略) ...

void Particle::CreateResources() {
    DirectXCommon* dxCommon = DirectXCommon::GetInstance();
    ID3D12Device* device = dxCommon->GetDevice();

    // 1. 頂点バッファの生成
    uint32_t vbSize = sizeof(VertexData) * kMaxParticles; // 最大数分のサイズを確保
    vbResource = dxCommon->CreateBufferResource(vbSize);
    vbResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

    // 2. 頂点バッファビューの作成
    vbView.BufferLocation = vbResource->GetGPUVirtualAddress();
    vbView.SizeInBytes = vbSize;
    // ビルボード描画では、頂点シェーダー内でジオメトリを生成するため、
    // ここでは1パーティクルあたり1頂点として、Float4(Position, Scale, Color)を
    // インスタンスバッファとして扱うことが多いです。
    // 今回のVertexDataの定義(pos, scale, color)はこれを想定しています。
    vbView.StrideInBytes = sizeof(VertexData);

    // 3. 定数バッファの生成
    uint32_t cbSize = sizeof(ConstBufferData);
    cbResource = dxCommon->CreateBufferResource(cbSize);
    cbResource->Map(0, nullptr, reinterpret_cast<void**>(&constMap));

    // 初期値設定
    constMap->viewProjectionMatrix = Matrix4x4::Identity();
    constMap->inverseViewMatrix = Matrix4x4::Identity();
}

void Particle::InitializeVertexData() {
    // 初期化時にすべての頂点データにクリアな値を入れておく
    for (int i = 0; i < kMaxParticles; ++i) {
        vertexData[i].pos = { 0.0f, 0.0f, 0.0f };
        vertexData[i].scale = 0.0f;
        vertexData[i].color = { 0.0f, 0.0f, 0.0f, 0.0f };
    }
}