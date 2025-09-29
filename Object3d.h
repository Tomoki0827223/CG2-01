// Object3d.h (新規作成)
#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <cstdint>
#include <string>
#include <vector>
#include "Vector4.h" // 頂点データ用
#include "Vector2.h" // 頂点データ用
#include "Vector3.h" // 頂点データ用
#include "Matrix4x4.h" // 変換行列用

// 前方宣言: Object3dCommonへのポインタを持つため
class Object3dCommon;

class Object3d
{
private:
    // --- Objファイル関連の構造体をインナークラスとして実装 ---

    // --- マテリアルデータ構造体 (ConstantBuffer) ---
    struct Material { //
        Vector4 color;
        int32_t enableLighting;
        float padding[3];
        Matrix4x4 uvTransform;
    };

    // --- 座標変換行列構造体 (ConstantBuffer) ---
    struct TransformationMatrix { //
        Matrix4x4 WVP;
        Matrix4x4 World;
    };

    // --- 平行光源構造体 (ConstantBuffer) ---
    // main.cppの定義（DirectionalLight）を参考に、ここでは最低限の構造を定義
    struct DirectionalLight { //
        Vector4 color;
        Vector3 direction;
        float intensity;
    };

    struct Transform {
        Vector3 scale;
        Vector3 rotate;
        Vector3 translate;
    };

public:
    
    // 頂点データ構造体
    struct VertexData {
        Vector4 position;
        Vector2 texcoord;
        Vector3 normal;
    };

    struct MaterialData { //
        std::string textureFilePath;
        uint32_t textureIndex = 0; // テクスチャ番号を保持
    };

    // モデルデータ構造体
    struct ModelData {
        std::vector<VertexData> vertices;
        MaterialData material;
    };
    
    // --- メンバ関数 ---
    // 初期化関数
    void Initialize(Object3dCommon* object3dCommon);
    void Update();
    void Draw();

    // --- Objファイル読み込み関数を静的メンバ関数として移植（main.cppからコピー） ---
    // 戻り値としてModelDataを返すため、インスタンスに依存しない静的関数とする
    static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
    static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename); //
    // ----------------------------------------------------------------------


    Transform transform;      // オブジェクトのスケール、回転、移動
    Transform cameraTransform; // カメラのスケール、回転、移動

private:
    // --- 共通部ポインタ ---
    Object3dCommon* object3dCommon = nullptr; //

    // --- Objファイルからのデータ ---
    ModelData modelData; //

    // --- 頂点データ管理 ---
    // Spriteと違い、Objファイルは頂点リストなのでIndexBufferは不要
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource; // VertexBuffer
    VertexData* vertexData = nullptr;                     // バッファ内のデータポインタ
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};           // VertexBufferView

    // --- マテリアルデータ管理 ---
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource; // マテリアルリソース
    Material* materialData = nullptr;                       // データポインタ

    // --- 座標変換行列データ管理 ---
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource; // 座標変換行列リソース
    TransformationMatrix* transformationMatrixData = nullptr;           // データポインタ

    // --- 平行光源データ管理 ---
    Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource; // 平行光源リソース
    DirectionalLight* directionalLightData = nullptr;                 // データポインタ


    // --- 内部処理関数 (リソース作成を private 関数に分割) ---
    void CreateVertexData(); // 頂点データ作成
    void CreateMaterialData(); // マテリアルリソース作成
    void CreateTransformationMatrixData(); // 座標変換行列リソース作成
    void CreateDirectionalLightData(); // 平行光源リソース作成

};