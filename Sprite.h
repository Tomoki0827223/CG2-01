#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <cstdint>
#include "Vector4.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include "affine.h"

// 前方宣言
class SpriteCommon;

// 頂点データ
struct VertexData {
    Vector4 position;
    Vector2 texcoord;
    Vector3 normal;
};

// マテリアルデータ
struct Material {
    Vector4 color;
    int32_t enableLighting;
    float padding[3];
    Matrix4x4 uvTransform;
};

struct TransformationMatrix
{
    Matrix4x4 WVP;
    Matrix4x4 World;

};

class Sprite
{
public:

    void Initialize(SpriteCommon* spriteCommon);
    void Update(); // ←追加
    void Draw();   // ←追加

private:
    // 共通部
    SpriteCommon* spriteCommon = nullptr;

    // バッファリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource; // VertexBuffer
    Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;  // IndexBuffer

    // バッファリソース内のデータを指すポインタ
    VertexData* vertexData = nullptr;
    uint32_t* indexData = nullptr;

    // バッファビュー
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
    D3D12_INDEX_BUFFER_VIEW indexBufferView{};

    // 頂点データ作成（初期化用のprivate関数）
    void CreateVertexData();
    
    // マテリアルリソース（ConstantBuffer）
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
    // バッファリソース内のデータを指すポインタ
    Material* materialData = nullptr;

    // マテリアルデータ作成（初期化用のprivate関数）
    void CreateMaterialData();

    // 座標変換行列リソース（ConstantBuffer）
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
    // バッファリソース内のデータを指すポインタ
    TransformationMatrix* transformationMatrixData = nullptr;

    // 座標変換行列データ作成（初期化用のprivate関数）
    void CreateTransformationMatrixData();
};