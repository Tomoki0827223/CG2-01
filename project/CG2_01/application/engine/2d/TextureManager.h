#include <string>
#include <vector>
#include <wrl.h>
#include <d3d12.h>
#include <unordered_map>
#include "externals/DirectXTex/DirectXTex.h"

class DirectXCommon;
class SrvManager;

class TextureManager
{
private:

    // テクスチャ1枚分のデータ
    struct TextureData {
        // std::string filePath; // ★削除：unordered_mapのキーにするため
        DirectX::TexMetadata metadata;
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;

        uint32_t srvIndex; // ★追加：SRVマネージャから確保したインデックス
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU; // ★変更：SRVマネージャから取得
        D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU; // ★変更：SRVマネージャから取得
    };

    // テクスチャデータの配列 (連想配列)
    std::unordered_map<std::string, TextureData> textureDatas; // ★変更：vectorからunordered_mapへ
    static TextureManager* instance;

    TextureManager() = default;
    ~TextureManager() = default;
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    // SRVManagerへのポインタを追加
    SrvManager* srvManager = nullptr; // ★追加


public:

    static TextureManager* GetInstance();
    void Finalize();

    DirectXCommon* dxCommon = nullptr;
    // 初期化関数の引数にSrvManager*を追加
    void Initialize(DirectXCommon* dxCommon_, SrvManager* srvManager_); // ★変更


    /// <summary>
    /// テクスチャファイルの読み込み
    /// </summary>
    /// <param name="filePath">テクスチャファイルのパス</param>
    /// <returns>画像イメージデータ</returns>
    void LoadTexture(const std::string& filePath);

    const DirectX::TexMetadata& GetMetaData(const std::string& filePath);
    // -------------------------------------

    // テクスチャ番号取得 -> ファイルパスでインデックスを取得するように変更
    uint32_t GetSrvIndexByFilePath(const std::string& filePath); // ★変更：関数名変更

    // GPUハンドル取得 -> ファイルパスでGPUハンドルを取得するように変更
    D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPUByFilePath(const std::string& filePath); // ★変更：関数名変更

};