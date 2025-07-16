#include <string>
#include <vector>
#include <wrl.h>
#include <d3d12.h>
#include "externals/DirectXTex/DirectXTex.h"

class DirectXCommon;

class TextureManager
{
private:

    // テクスチャ1枚分のデータ
    struct TextureData {
        std::string filePath;
        DirectX::TexMetadata metadata;
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
        D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
    };

    // テクスチャデータの配列
    std::vector<TextureData> textureDatas;

    static TextureManager* instance;
    TextureManager() = default;
    ~TextureManager() = default;
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;


    // SRVインデックスの開始番号
    static uint32_t kSRVIndexTop;


public:

    static TextureManager* GetInstance();
    void Finalize();

    DirectXCommon* dxCommon = nullptr;
    void Initialize(DirectXCommon* dxCommon_);


    /// <summary>
    /// テクスチャファイルの読み込み
    /// </summary>
    /// <param name="filePath">テクスチャファイルのパス</param>
    /// <returns>画像イメージデータ</returns>
    void LoadTexture(const std::string& filePath);

    // テクスチャ番号取得
    uint32_t GetTextureIndexByFilePath(const std::string& filePath);
    // GPUハンドル取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);

};
