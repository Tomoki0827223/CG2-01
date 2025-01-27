#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "affine.h"
#include "SpriteCommon.h"

struct VertexData
{
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

//マテリアルデータ
struct Material
{
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

//座標変換行列データ
struct TransfomaitionMatrix
{
	Matrix4x4 WVP;
	Matrix4x4 world;
};

class Sprite
{
public:

	void Initialize(SpriteCommon* spriteCommon);
	void SetTexture(ID3D12Resource* texture, D3D12_GPU_DESCRIPTOR_HANDLE srvHandle);
	void Update();
	void Draw();

private:
	
	SpriteCommon* spriteCommon_ = nullptr;

	//バファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer_;
	//マテリアルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialBuffer_;
	//座標変換行列リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixBuffer_;

	//バファリソース内のデータを指すポインタ
	VertexData* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;

	//マテリアルデータを指すポインタ
	Material* materialData_ = nullptr;

	//バファリソース内のデータを指すポインタ
	TransfomaitionMatrix* transformationMatrixData_ = nullptr;

	//バファリソースの使い道を補足するバファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;

	uint32_t vertexCount_ = 4;
	uint32_t indexCount_ = 6;

	ID3D12Resource* texture_ = nullptr;
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle_ = {};

};

