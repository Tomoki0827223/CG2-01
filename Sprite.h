#pragma once
#include <d3d12.h>
#include "Matrix4x4.h"
#include <cstdint>
#include "Vector2.h"
#include "Vector4.h"
#include "Vector3.h"
#include <wrl.h>

using Microsoft::WRL::ComPtr;

// 頂点データ構造体
struct VertexData {
    Vector4 position;
    Vector2 texcoord;
    Vector3 normal;
};

// マテリアルデータ構造体
struct Material {
    Vector4 color;
    int32_t enableLighting;
    float padding[3];
    Matrix4x4 uvTransform;
};

// 座標変換行列構造体
struct TransformationMatrix {
    Matrix4x4 WVP;  // ワールド・ビュー・プロジェクション行列
    Matrix4x4 World; // ワールド行列
};

class Sprite {
public:
    Sprite();
    ~Sprite();

    void Initialize(ID3D12Device* device);

    const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return vertexBufferView; }
    const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const { return indexBufferView; }

private:
    // 頂点・インデックスバッファのリソース
    ComPtr<ID3D12Resource> vertexResource;
    ComPtr<ID3D12Resource> indexResource;

    // 頂点バッファビューとインデックスバッファビュー
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;

    // 頂点データとインデックスデータ
    VertexData* vertexData = nullptr;
    uint32_t* indexData = nullptr;
    Material* materialData = nullptr;

    // 座標変換行列用のバッファリソース
    ComPtr<ID3D12Resource> transformationMatrixResource;
    TransformationMatrix* transformationMatrixData = nullptr;

    // 頂点データ作成関数
    void CreateVertexData(ID3D12Device* device);
    void CreateIndexData(ID3D12Device* device);
    void CreateTransformationMatrix(ID3D12Device* device); // 座標変換行列データ作成
};
