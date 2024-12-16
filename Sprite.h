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
class WinApp;
class Sprite
{
public:


	void Initialize(SpriteCommon* spriteCommon);
	void Update();
	void Draw();

	D3D12_GPU_DESCRIPTOR_HANDLE* GetTextureSrvHandleGPU() { return &textureSrvHandleGPU; }
	TransformationMatrix* GetTransformationMatrixData() { return transformationMatrixData; }
	VertexData* GetVertexData() { return vertexData; }
	Material* GetMaterialData() { return materialData; }
	Microsoft::WRL::ComPtr<ID3D12Resource> GetVertexResource() { return vertexResource; }
	Microsoft::WRL::ComPtr<ID3D12Resource> GetIndexResource() { return indexResource; }
	Microsoft::WRL::ComPtr<ID3D12Resource> GetMaterialResource() { return materialResource; }
	Microsoft::WRL::ComPtr<ID3D12Resource> GetTransformationMatrixResource() { return transformationMatrixResource; }

private:

	Sprite* sprite = nullptr;
	WinApp* winApp_ = nullptr;
	SpriteCommon* spriteCommon_ = nullptr;

	//頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	//インデックスリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;

	//バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	//バファリソース内のデータを指すポインタ
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	//マテリアルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Material* materialData = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	TransformationMatrix* transformationMatrixData = nullptr;

	TransformVector3 transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	TransformVector3 cameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	TransformVector3 transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	// Add the missing textureSrvHandleGPU
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU;
};