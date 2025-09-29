#include "Sprite.h"
#include "SpriteCommon.h"
#include "DirectXCommon.h"
#include "TextureManager.h"

void Sprite::Initialize(SpriteCommon* spriteCommon, std::string textureFilePath) {

    this->spriteCommon = spriteCommon;

    // 1. テクスチャインデックスを取得
    textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

    // 2. 頂点データ、マテリアルデータ、座標変換行列データ作成 (既存の処理)
    CreateVertexData();
    CreateMaterialData();
    CreateTransformationMatrixData();

    // --- [🚨 修正箇所 3: Initialize内でAdjustTextureSizeを呼び出す 🚨] ---
    // テクスチャサイズを画像に合わせて設定
    AdjustTextureSize();
    // ----------------------------------------------------------------------
}

// --- [🚨 追加箇所 2: AdjustTextureSizeの実装 🚨] ---
void Sprite::AdjustTextureSize()
{
    // テクスチャメタデータを取得
    const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureIndex);

    // 切り出しサイズを画像サイズに合わせる
    // (textureSize が切り出しサイズとスプライトの描画サイズの両方を兼ねていると仮定)
    textureSize.x = static_cast<float>(metadata.width);
    textureSize.y = static_cast<float>(metadata.height);

    // ※ スライドの指示 に従って textureSize を設定
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
    //vertexData[0].position = Vector4(-0.5f, 0.5f, 0.0f, 1.0f); // 左上
    //vertexData[0].texcoord = Vector2(0.0f, 0.0f);
    //vertexData[0].normal = Vector3(0.0f, 0.0f, 1.0f);

    //vertexData[1].position = Vector4(0.5f, 0.5f, 0.0f, 1.0f); // 右上
    //vertexData[1].texcoord = Vector2(1.0f, 0.0f);
    //vertexData[1].normal = Vector3(0.0f, 0.0f, 1.0f);

    //vertexData[2].position = Vector4(-0.5f, -0.5f, 0.0f, 1.0f); // 左下
    //vertexData[2].texcoord = Vector2(0.0f, 1.0f);
    //vertexData[2].normal = Vector3(0.0f, 0.0f, 1.0f);

    //vertexData[3].position = Vector4(0.5f, -0.5f, 0.0f, 1.0f); // 右下
    //vertexData[3].texcoord = Vector2(1.0f, 1.0f);
    //vertexData[3].normal = Vector3(0.0f, 0.0f, 1.0f);

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
    // --- 1. 頂点座標の計算 (アンカーポイントとフリップの反映) ---

    // 頂点座標の計算 (幅1.0f, 高さ1.0fのスプライトとして計算)
    float left = 0.0f - anchorPoint.x;
    float right = 1.0f - anchorPoint.x;
    float top = 1.0f - anchorPoint.y;
    float bottom = 0.0f - anchorPoint.y;

    // isFlipX/Y に応じて座標を入れ替え
    if (isFlipX) {
        std::swap(left, right);
    }
    if (isFlipY) {
        std::swap(top, bottom);
    }

    // --- 2. 描画サイズの調整 (アスペクト比の計算とウィンドウ補正) ---

    const float kDefaultSizeNDC = 0.5f; // 描画サイズ基準 (NDC座標系)
    const DirectX::TexMetadata& metadata =
        TextureManager::GetInstance()->GetMetaData(textureIndex);

    // 1. テクスチャの解像度に基づくアスペクト比
    float textureAspectRatio = (float)metadata.width / (float)metadata.height;

    // 2. ウィンドウの解像度に基づくアスペクト比 (WinApp::kClientWidth/kClientHeight にアクセスできると仮定)
    // ※ この情報がなければ正方形にはできません。
    // ※ WinApp.hがインクルードされていないため、エラーになる場合は適宜修正してください。
    float windowAspectRatio = (float)WinApp::kClientWidth / (float)WinApp::kClientHeight;

    // NDCサイズ計算 (テクスチャのアスペクト比を反映)
    float kSpriteSizeX = kDefaultSizeNDC;
    float kSpriteSizeY = kDefaultSizeNDC;

    if (textureAspectRatio > 1.0f) { // 横長テクスチャの場合
        kSpriteSizeY = kDefaultSizeNDC / textureAspectRatio;
    }
    else { // 縦長または正方形テクスチャの場合
        kSpriteSizeX = kDefaultSizeNDC * textureAspectRatio;
    }

    // 3. ウィンドウのアスペクト比でY軸を補正 (正方形に見えるようにする)
    kSpriteSizeY *= windowAspectRatio;

    // --- 3. 頂点位置の更新 ---

    // 調整した kSpriteSizeX/Y を適用してNDC座標を設定
    vertexData[0].position = Vector4(left * kSpriteSizeX, top * kSpriteSizeY, 0.0f, 1.0f); // 左上
    vertexData[1].position = Vector4(right * kSpriteSizeX, top * kSpriteSizeY, 0.0f, 1.0f);  // 右上
    vertexData[2].position = Vector4(left * kSpriteSizeX, bottom * kSpriteSizeY, 0.0f, 1.0f);// 左下
    vertexData[3].position = Vector4(right * kSpriteSizeX, bottom * kSpriteSizeY, 0.0f, 1.0f); // 右下

    // --- 4. テクスチャ座標 (UV) の計算と更新 ---

    // ピクセル座標から正規化UV座標を計算
    float tex_left = textureLeftTop.x / metadata.width;
    float tex_right = (textureLeftTop.x + textureSize.x) / metadata.width;
    float tex_top = textureLeftTop.y / metadata.height;
    float tex_bottom = (textureLeftTop.y + textureSize.y) / metadata.height;

    // 頂点データのtexcoordを更新
    vertexData[0].texcoord = Vector2(tex_left, tex_top);    // 左上
    vertexData[1].texcoord = Vector2(tex_right, tex_top);   // 右上
    vertexData[2].texcoord = Vector2(tex_left, tex_bottom); // 左下
    vertexData[3].texcoord = Vector2(tex_right, tex_bottom);// 右下

    // --- 5. 座標変換行列の更新 ---

    // position_で移動
    Matrix4x4 worldMatrix = MakeTranslateMatrix(Vector3(position_.x, position_.y, 0.0f));

    Matrix4x4 viewMatrix = MakeIdentity4x4();
    Matrix4x4 projectionMatrix = MakeIdentity4x4();

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

    // SRVDescriptorTableの先頭を設定（変更点）
    commandList->SetGraphicsRootDescriptorTable(
        2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex)
    );

    // 描画コール
    commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::ChangeTexture(const std::string& textureFilePath) {
    // テクスチャが未ロードならロード
    TextureManager::GetInstance()->LoadTexture(textureFilePath);
    // テクスチャ番号を更新
    textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);
}
