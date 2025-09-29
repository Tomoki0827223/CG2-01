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
    dxCommon = dxCommon_; // 👈 ここでポインタを設定
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

    // [🚨 修正箇所 1: アップロード処理の実行とコマンドリストの処理を追加 🚨]
    // 6. テクスチャのアップロードとコマンドリストの実行

    // コマンドリストをリセットしてテクスチャアップロードコマンドを記録可能にする (直前で閉じられていなければOK)
    // 実際には、Initialize中に全てのテクスチャをロードする場合は、
    // 全ロード後に一度に実行するのが効率的ですが、今回は即座にアップロードします。
    // Initialize()時にコマンドリストは開かれていると仮定します。

    // アップロード用の中間リソースを取得
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = dxCommon->UploadTextureData(textureData.resource, mipImages);

    // [一時的な処理] ロード後にコマンドリストをクローズして実行し、リセットする
    // Initialize中でテクスチャをロードしているため、この処理が必要です。
    // 通常は、Initializeの最後に一度だけ実行します。

    HRESULT hr_close = dxCommon->GetCommandList()->Close();
    assert(SUCCEEDED(hr_close));

    ID3D12CommandList* commandLists[] = { dxCommon->GetCommandList() };
    dxCommon->GetCommandQueue()->ExecuteCommandLists(_countof(commandLists), commandLists);

    // フェンスでGPUの完了を待機
    dxCommon->WaitForGPU();

    // コマンドリストをリセットし、再びコマンドを記録できるようにする
    HRESULT hr_reset = dxCommon->GetCommandAllocator()->Reset();
    assert(SUCCEEDED(hr_reset));
    hr_reset = dxCommon->GetCommandList()->Reset(dxCommon->GetCommandAllocator(), nullptr);
    assert(SUCCEEDED(hr_reset));

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

// --- [追加箇所：メタデータ取得関数の実装] ---
const DirectX::TexMetadata& TextureManager::GetMetaData(uint32_t textureIndex) {
    // 範囲外指定違反チェック
    assert(textureIndex < textureDatas.size());

    // テクスチャデータの参照を取得し、メタデータを返す
    const TextureData& textureData = textureDatas[textureIndex];
    return textureData.metadata;
}
// ---------------------------------------------