#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <cstdint>
#include <string>
#include <vector>
#include "Vector4.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include "ModelCommon.h" // ModelCommonクラスの定義が必要

// 前方宣言
class ModelCommon;

class Model
{
private:
    // --- ConstantBufferの構造体定義 (Object3d.hから移行) ---
    struct Material {
        Vector4 color;
        int32_t enableLighting;
        float padding[3];
        Matrix4x4 uvTransform;
    };

public:
    // 頂点データ構造体 (Object3d.hから移行)
    struct VertexData {
        Vector4 position;
        Vector2 texcoord;
        Vector3 normal;
    };

    // マテリアルデータ構造体 (Object3d.hから移行)
    struct MaterialData {
        std::string textureFilePath;
        uint32_t textureIndex = 0; // テクスチャ番号を保持
    };

    // モデルデータ構造体 (Object3d.hから移行)
    struct ModelData {
        std::vector<VertexData> vertices;
        MaterialData material;
    };

    // --- メンバ関数 ---
    void Initialize(ModelCommon* modelCommon); // 初期化
    void Draw();                               // 描画

    // --- Objファイル読み込み関数を静的メンバ関数として移植（Object3d.hから移行） ---
    static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
    static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);


private:
    // --- 共通部ポインタ ---
    ModelCommon* modelCommon_ = nullptr; //

    // --- Objファイルからのデータ ---
    ModelData modelData; //

    // --- 頂点データ管理 ---
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource; // VertexBuffer
    VertexData* vertexData = nullptr;                     // バッファ内のデータポインタ
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};           // VertexBufferView

    // --- マテリアルデータ管理 ---
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource; // マテリアルリソース
    Material* materialData = nullptr;                       // データポインタ

    // --- 内部処理関数 (リソース作成を private 関数に分割) ---
    void CreateVertexData();
    void CreateMaterialData();
};