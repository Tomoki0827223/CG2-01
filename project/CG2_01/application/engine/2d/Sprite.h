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

    void Initialize(SpriteCommon* spriteCommon, std::string textureFilePath);
    void Update();
    void Draw();
    void ChangeTexture(const std::string& textureFilePath); // ←追加
    Vector2 position_{ 0.0f, 0.0f };
    uint32_t textureIndex = 0;

    std::string filePath_; // ★追加: ファイルパスを保持

    // --- [🚨 修正・追加箇所 1: メンバ変数とGetter/Setter 🚨] ---
    // メンバ変数
    Vector2 anchorPoint = { 0.5f, 0.5f }; // アンカーポイント (デフォルトは中心)
    bool isFlipX = false;                 // 左右フリップ
    bool isFlipY = false;                 // 上下フリップ

    // --- [追加箇所：テクスチャ範囲指定用のメンバ変数] ---
    Vector2 textureLeftTop = { 0.0f, 0.0f }; // テクスチャ左上座標 (ピクセル単位)
    Vector2 textureSize = { 100.0f, 100.0f }; // テクスチャ切り出しサイズ (ピクセル単位)
    // ----------------------------------------------------

    // Getter
    const Vector2& GetAnchorPoint() const { return anchorPoint; }
    bool GetFlipX() const { return isFlipX; }
    bool GetFlipY() const { return isFlipY; }

    // --- [追加箇所：テクスチャ範囲指定用のGetter] ---
    const Vector2& GetTextureLeftTop() const { return textureLeftTop; }
    const Vector2& GetTextureSize() const { return textureSize; }
    // ----------------------------------------------------

    // Setter
    void SetAnchorPoint(const Vector2& anchorPoint) { this->anchorPoint = anchorPoint; }
    void SetFlipX(bool isFlip) { isFlipX = isFlip; }
    void SetFlipY(bool isFlip) { isFlipY = isFlip; }

    // --- [追加箇所：テクスチャ範囲指定用のSetter] ---
    void SetTextureLeftTop(const Vector2& leftTop) { this->textureLeftTop = leftTop; }
    void SetTextureSize(const Vector2& size) { this->textureSize = size; }
    // ----------------------------------------------------

    // --- [🚨 追加箇所 1: AdjustTextureSizeの宣言 🚨] ---
    // テクスチャサイズをイメージに合わせる
    void AdjustTextureSize();
    // ----------------------------------------------------

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