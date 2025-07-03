#include "Sprite.h"
#include "SpriteCommon.h"
#include "DirectXCommon.h"

void Sprite::Initialize(SpriteCommon* spriteCommon, std::string textureFilePath)
{
    this->spriteCommon = spriteCommon;

    // 頂点データ作成
    CreateVertexData();

    // マテリアルデータ作成
    CreateMaterialData();

    // 座標変換行列データ作成
    CreateTransformationMatrixData();
    
    textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);
}

void Sprite::CreateVertexData()
{
    // デバイス取得
    auto dxCommon = spriteCommon->GetDXCommon();
    auto device = dxCommon->GetDevice();

    // 頂点数・インデックス数
    const size_t vertexCount = 4;
    const size_t indexCount = 6;

    // VertexResourceを作る
    vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * vertexCount);

    // IndexResourceを作る
    indexResource = dxCommon->CreateBufferResource(sizeof(uint32_t) * indexCount);

    // VertexBufferViewを作成する（値を設定するだけ）
    vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * vertexCount);
    vertexBufferView.StrideInBytes = sizeof(VertexData);

    // IndexBufferViewを作成する（値を設定するだけ）
    indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
    indexBufferView.SizeInBytes = UINT(sizeof(uint32_t) * indexCount);
    indexBufferView.Format = DXGI_FORMAT_R32_UINT;

    // VertexResourceにデータを書き込むためのアドレスを取得してvertexDataに割り当てる
    vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

    // IndexResourceにデータを書き込むためのアドレスを取得してindexDataに割り当てる
    indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

    // 頂点データの初期値をセット
    // 左上(0), 右上(1), 左下(2), 右下(3) の順
    vertexData[0].position = Vector4(-0.5f, 0.5f, 0.0f, 1.0f); // 左上
    vertexData[0].texcoord = Vector2(0.0f, 0.0f);
    vertexData[0].normal = Vector3(0.0f, 0.0f, 1.0f);

    vertexData[1].position = Vector4(0.5f, 0.5f, 0.0f, 1.0f); // 右上
    vertexData[1].texcoord = Vector2(1.0f, 0.0f);
    vertexData[1].normal = Vector3(0.0f, 0.0f, 1.0f);

    vertexData[2].position = Vector4(-0.5f, -0.5f, 0.0f, 1.0f); // 左下
    vertexData[2].texcoord = Vector2(0.0f, 1.0f);
    vertexData[2].normal = Vector3(0.0f, 0.0f, 1.0f);

    vertexData[3].position = Vector4(0.5f, -0.5f, 0.0f, 1.0f); // 右下
    vertexData[3].texcoord = Vector2(1.0f, 1.0f);
    vertexData[3].normal = Vector3(0.0f, 0.0f, 1.0f);

    // インデックスデータの初期値をセット（2三角形）
    indexData[0] = 0; // 左上
    indexData[1] = 1; // 右上
    indexData[2] = 2; // 左下
    indexData[3] = 2; // 左下
    indexData[4] = 1; // 右上
    indexData[5] = 3; // 右下
}

void Sprite::CreateMaterialData()
{
    // デバイス取得
    auto dxCommon = spriteCommon->GetDXCommon();
    auto device = dxCommon->GetDevice();

    // マテリアルリソースを作る
    materialResource = dxCommon->CreateBufferResource(sizeof(Material));

    // マテリアルリソースにデータを書き込むためのアドレスを取得してmaterialDataに割り当てる
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

    // マテリアルデータの初期値を書き込む
    materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData->enableLighting = false;
    materialData->uvTransform = MakeIdentity4x4();
}

void Sprite::CreateTransformationMatrixData()
{
    // デバイス取得
    auto dxCommon = spriteCommon->GetDXCommon();
    auto device = dxCommon->GetDevice();

    // 座標変換行列リソースを作る
    transformationMatrixResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));

    // 座標変換行列リソースにデータを書き込むためのアドレスを取得してtransformationMatrixDataに割り当てる
    transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));

    // 単位行列を書きこんでおく
    transformationMatrixData->WVP = MakeIdentity4x4();
    transformationMatrixData->World = MakeIdentity4x4();
}

void Sprite::Update()
{
    // 頂点リソースにデータを書き込む（4点分）
    // 例: ここではY座標をアニメーションさせる
    float time = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count()) * 0.001f;
    float offsetY = std::sin(time) * 0.1f; // 上下に0.1動かす

    vertexData[0].position.y = 0.5f + offsetY;  // 左上
    vertexData[1].position.y = 0.5f + offsetY;  // 右上
    vertexData[2].position.y = -0.5f + offsetY; // 左下
    vertexData[3].position.y = -0.5f + offsetY; // 右下

    // インデックスリソースにデータを書き込む（6個分）
    // 例: 通常は固定だが、例えば左右を入れ替える場合
    // indexData[0] = 1; // 右上
    // indexData[1] = 0; // 左上
    // indexData[2] = 2; // 左下
    // indexData[3] = 2; // 左下
    // indexData[4] = 0; // 左上
    // indexData[5] = 3; // 右下

    // Transform情報を作る
    Matrix4x4 worldMatrix = MakeIdentity4x4();
    // ここで平行移動や回転などを合成する場合は行列を掛け合わせてworldMatrixを作る

    // ViewMatrixを作って単位行列を代入（2Dスプライトなら単位行列でOK）
    Matrix4x4 viewMatrix = MakeIdentity4x4();

    // ProjectionMatrixを作って並行投影行列を書き込む
    Matrix4x4 projectionMatrix = MakeIdentity4x4();
    // 必要に応じて正射影行列を作成する関数を使ってください

    // transformationMatrixDataに書き込む
    transformationMatrixData->WVP = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
    transformationMatrixData->World = worldMatrix;
}

void Sprite::Draw()
{
    // コマンドリスト取得
    auto dxCommon = spriteCommon->GetDXCommon();
    auto commandList = dxCommon->GetCommandList();

    // VertexBufferView/IndexBufferViewの設定
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
    commandList->IASetIndexBuffer(&indexBufferView);

    // マテリアルCBV, 座標変換CBVの設定
    commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
    commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

    // SRVのDescriptorTableの先頭を設定（ここを修正）
    commandList->SetGraphicsRootDescriptorTable(
        2, // SRVのRootParameterインデックス
        TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex)
    );

    // 描画コール
    commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}