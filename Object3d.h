#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <wrl.h>
#include <d3d12.h>
#include "Matrix4x4.h"

#include "affine.h"
#include "Object3dCommon.h"

class Object3dCommon; // 前方宣言

// 頂点データ
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

// 座標変換行列データ
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

struct MaterialData {
	std::string textureFilePath;
};


struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

// マテリアルデータ
struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

// 平行光源データ
struct DirectionalLight {
	Vector3 direction;   // 光の向き（正規化ベクトル）
	float pad1;          // パディング（16バイトアライメント用）
	Vector3 color;       // 光の色（RGB）
	float intensity;     // 光の強さ
};

struct MaterialData {
	std::string textureFilePath;
	uint32_t textureIndex = 0;
};

class Object3d
{
public:

	//.mtlファイルの読み取り
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
	//.objファイルの読み取り
	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

	// 初期化	
	void Initialize(Object3dCommon* object3dCommon);

private:
	
	Object3dCommon* object3dCommon_ = nullptr;
	ModelData modelData;

	// 頂点バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	// バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	
	// マテリアルリソース（定数バッファ）
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = nullptr;
	// バッファリソース内のデータを指すポインタ
	Material* materialData = nullptr;


	// 座標変換行列リソース（定数バッファ）
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource = nullptr;
	// バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData = nullptr;


	// 平行光源リソース（定数バッファ）
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = nullptr;
	// バッファリソース内のデータを指すポインタ
	DirectionalLight* directionalLightData = nullptr;
};

