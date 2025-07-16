#include "TextureManager.h"
#include "DirectXCommon.h"


TextureManager* TextureManager::instance = nullptr;

// ImGuiで0番を使用するため、1番から使用
uint32_t TextureManager::kSRVIndexTop = 1;

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filePath) {
    auto it = std::find_if(textureDatas.begin(), textureDatas.end(),
        [&](const TextureData& data) { return data.filePath == filePath; });
    if (it != textureDatas.end()) {
        uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
        return textureIndex;
    }
    assert(0); // 見つからなければ停止

    return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex) {
    assert(textureIndex < textureDatas.size());

	TextureData& textureData = textureDatas[textureIndex];
	return textureData.srvHandleGPU;
}

TextureManager* TextureManager::GetInstance() {
    if (instance == nullptr) {
        instance = new TextureManager;
    }
    return instance;
}

void TextureManager::Finalize() {
    delete instance;
    instance = nullptr;
}

void TextureManager::Initialize(DirectXCommon* dxCommon_) {
    dxCommon = dxCommon_;
    textureDatas.reserve(DirectXCommon::kMaxSRVCount);
}

void TextureManager::LoadTexture(const std::string& filePath)
{
    // 1. 読み込み済みテクスチャの検索
    auto it = std::find_if(
        textureDatas.begin(),
        textureDatas.end(),
        [&filePath](const TextureData& textureData) {
            return textureData.filePath == filePath;
        }
    );
    if (it != textureDatas.end()) {
        return; // 既に読み込み済みならreturn
    }

    // 2. 最大数チェック
    assert(textureDatas.size() + kSRVIndexTop < DirectXCommon::kMaxSRVCount);

    // 3. デスクリプタハンドルの計算
    textureDatas.resize(textureDatas.size() + 1);
    TextureData& textureData = textureDatas.back();
    uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1) + kSRVIndexTop;

    // 4. ファイル読み込み・MipMap作成
    std::wstring filePathW = StringUitilty::ConvertString(filePath);
    DirectX::ScratchImage image{};
    HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
    assert(SUCCEEDED(hr));
    DirectX::ScratchImage mipImages{};
    hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
    assert(SUCCEEDED(hr));

    // 5. テクスチャデータ書き込み
    textureData.filePath = filePath;
    textureData.metadata = mipImages.GetMetadata();
    textureData.resource = dxCommon->CreateTextureResource(
        dxCommon->GetDevice(), textureData.metadata
    );

    textureData.srvHandleCPU = dxCommon->GetSRVCPUDescriptorHandle(srvIndex);
    textureData.srvHandleGPU = dxCommon->GetSRVGPUDescriptorHandle(srvIndex);

    // 7. SRV生成
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = textureData.metadata.format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = static_cast<UINT>(textureData.metadata.mipLevels);
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    
    dxCommon->GetDevice()->CreateShaderResourceView(
        textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU
    );
}