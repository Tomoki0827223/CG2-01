#include "TextureManager.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "StringUitilty.h"

TextureManager* TextureManager::instance = nullptr;

// ファイルパスからSRVインデックスを取得
uint32_t TextureManager::GetSrvIndexByFilePath(const std::string& filePath) {
    // unordered_mapから検索し、SRVインデックスを返す
    assert(textureDatas.contains(filePath)); // 見つからなければ停止
    return textureDatas.at(filePath).srvIndex;
}

// ファイルパスからGPUハンドルを取得
D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPUByFilePath(const std::string& filePath) {
    assert(textureDatas.contains(filePath));

	TextureData& textureData = textureDatas.at(filePath);
    // SRVManagerから再計算しても良いが、キャッシュされた値を返す
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

// Initialize関数の変更
void TextureManager::Initialize(DirectXCommon* dxCommon_, SrvManager* srvManager_) {
    dxCommon = dxCommon_;
    srvManager = srvManager_; // ★変更：SrvManagerのポインタを記録
    // textureDatas.reserve(DirectXCommon::kMaxSRVCount); // reserveはmapでは不要だが、古いコードがあれば削除
}


void TextureManager::LoadTexture(const std::string& filePath)
{
    // 1. 読み込み済みテクスチャの検索 (unordered_map用に変更)
    if (textureDatas.contains(filePath)) { // ★変更
        return; // 既に読み込み済みならreturn
    }

    // 2. 最大数チェック (SrvManagerに委譲)
    assert(srvManager->CanAllocate()); // ★変更

    // 3. デスクリプタハンドルの計算とデータ挿入
    uint32_t srvIndex = srvManager->Allocate();
    TextureData& textureData = textureDatas[filePath];
    textureData.srvIndex = srvIndex;

    // SRVManagerからハンドルを取得
    textureData.srvHandleCPU = srvManager->GetCPUDescriptorHandle(srvIndex);
    textureData.srvHandleGPU = srvManager->GetGPUDescriptorHandle(srvIndex);


    // 4. ファイル読み込み・MipMap作成
    std::wstring filePathW = StringUitilty::ConvertString(filePath);
    DirectX::ScratchImage image{};
    HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
    assert(SUCCEEDED(hr));
    DirectX::ScratchImage mipImages{};
    hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
    assert(SUCCEEDED(hr));

    // 5. テクスチャデータ書き込み
    // textureData.filePath = filePath; // ★削除：mapのキーなので不要
    textureData.metadata = mipImages.GetMetadata();
    textureData.resource = dxCommon->CreateTextureResource(
        dxCommon->GetDevice(), textureData.metadata
    );

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
    
    // CreateShaderResourceViewの呼び出しは、確保したハンドルを使用
    dxCommon->GetDevice()->CreateShaderResourceView(
        textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU
    );
}

// ファイルパスからメタデータを取得する (TextureManager.hのuint32_tの引数とは型が異なる)
const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath) {
    assert(textureDatas.contains(filePath));
    return textureDatas.at(filePath).metadata;
}