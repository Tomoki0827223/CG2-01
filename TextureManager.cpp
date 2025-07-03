#include "TextureManager.h"
#include "DirectXCommon.h" // kMaxSRVCount を使うため

TextureManager* TextureManager::instance = nullptr;
// ImGuiで0番を使用するため、1番から使用
uint32_t TextureManager::kSRVIndexTop = 1;

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex) {
    // 範囲外指定をチェック
    assert(textureIndex < textureDatas.size());
    TextureData& textureData = textureDatas[textureIndex];
    return textureData.srvHandleGPU;
}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filePath) {
    // 読み込み済みテクスチャデータを検索
    auto it = std::find_if(
        textureDatas.begin(),
        textureDatas.end(),
        [&filePath](const TextureData& textureData) {
            return textureData.filePath == filePath;
        }
    );
    if (it != textureDatas.end()) {
        // 読み込み済みなら要素番号を返す
        uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
        return textureIndex;
    }
    assert(0);
    return 0;
}


TextureManager* TextureManager::GetInstance() {
    if (instance == nullptr) {
        instance = new TextureManager;
    }
    return instance;
}

void TextureManager::Initialize() {
    // SRVの数と同じだけメモリを確保
    textureDatas.reserve(DirectXCommon::kMaxSRVCount);
}

void TextureManager::Finalize() {
    delete instance;
    instance = nullptr;
}

void TextureManager::LoadTexture(const std::string& filePath) {

    // 既に読み込み済みのテクスチャを検索
    auto it = std::find_if(
        textureDatas.begin(),
        textureDatas.end(),
        [&filePath](const TextureData& textureData) {
            return textureData.filePath == filePath;
        }
    );
    if (it != textureDatas.end()) {
        // 読み込み済みなら何もせずreturn

        // 最大テクスチャ枚数上限チェック
        assert(textureDatas.size() + kSRVIndexTop < DirectXCommon::kMaxSRVCount);

        return;
    }

    // テクスチャデータを追加
    textureDatas.resize(textureDatas.size() + 1);
    TextureData& textureData = textureDatas.back();

    // ファイルパスを保存
    textureData.filePath = filePath;

    DirectX::TexMetadata metadata;
    DirectX::ScratchImage image;
    HRESULT hr = DirectX::LoadFromWICFile(
        filePath, // const wchar_t*
        DirectX::WIC_FLAGS_NONE,
        &metadata,
        image
    );

    // MipMapの作成
    DirectX::ScratchImage mipImages{};
    DirectX::GenerateMipMaps(
        image.GetImages(), image.GetImageCount(), image.GetMetadata(),
        DirectX::TEX_FILTER_DEFAULT, 0, mipImages
    );

    // DirectXCommonのインスタンス取得（シングルトン等で取得する想定）
    extern DirectXCommon* dxCommon; // グローバル変数として宣言されている場合
    ID3D12Device* device = dxCommon->GetDevice();

    // テクスチャリソース生成
    textureData.resource = dxCommon->CreateTextureResource(device, textureData.metadata);

    // テクスチャデータ転送
    dxCommon->UploadTextureData(textureData.resource, mipImages);

    uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1) + kSRVIndexTop;

    // デスクリプタハンドル取得
    textureData.srvHandleCPU = dxCommon->GetSRVCPUDescriptorHandle(srvIndex);
    textureData.srvHandleGPU = dxCommon->GetSRVGPUDescriptorHandle(srvIndex);

    // SRVの設定
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = textureData.metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);

    // SRV生成
    device->CreateShaderResourceView(
        textureData.resource.Get(),
        &srvDesc,
        textureData.srvHandleCPU
    );
}