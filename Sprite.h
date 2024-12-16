#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "affine.h"
#include <wrl.h>
#include <d3d12.h>
#include <cstdint>

struct VertexData
{
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct Material
{
	Vector4 color;
	int32_t endleLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

struct TransformationMatrix
{
	Matrix4x4 WVP;
	Matrix4x4 world;

};



class SpriteCommon;
class Sprite
{
public:

	~Sprite();
	void Initialize(SpriteCommon* spriteCommon);
	void Update();
	void Draw();

private:

	Sprite* sprite = nullptr;

	SpriteCommon* spriteCommon_ = nullptr;

	//頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	//インデックスリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;

	//VertexBufferResourceを生成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite;
	//IndexBufferResourceを生成
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite;

	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	
	//頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	//インデックスリソースを作る
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	//マテリアルリソースを作る//Material用のResourceを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	//マテリアルのポインター
	Material* materialData = nullptr;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> materialHeap;

	//バッファリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	//バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrix = nullptr;


};