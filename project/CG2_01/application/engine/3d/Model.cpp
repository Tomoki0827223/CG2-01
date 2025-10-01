#include "Model.h"
#include "TextureManager.h"
#include "DirectXCommon.h"
#include "affine.h"
#include <fstream>
#include <sstream>
#include <cassert>
#include <algorithm>
#include "Object3d.h"
#include "ModelManager.h"

// --- LoadMaterialTemplateFileの定義 (Object3d.cppから移植) ---
Model::MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
    Model::MaterialData materialData;
    // ... (処理はObject3d.cppの定義と同じ) ...
    std::string line;
    std::string fullPath = directoryPath + "/" + filename;
    std::ifstream file(fullPath);
    assert(file.is_open());

    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;

        if (identifier == "map_Kd") {
            std::string textureFileName;
            s >> textureFileName;
            materialData.textureFilePath = directoryPath + "/" + textureFileName;
        }
    }
    return materialData;
}

// --- LoadObjFileの定義 (Object3d.cppから移植。テクスチャ反転処理は削除) ---
Model::ModelData Model::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
    Model::ModelData modelData;
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> texcoords;
    // ... (処理はObject3d.cppの定義と同じ) ...
    std::string line;
    std::string fullPath = directoryPath + "/" + filename;
    std::ifstream file(fullPath);

    assert(file.is_open());

    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;

        if (identifier == "v") {
            Vector3 position;
            s >> position.x >> position.y >> position.z;
            positions.push_back(position);
        }
        else if (identifier == "vt") {
            Vector2 texcoord;
            s >> texcoord.x >> texcoord.y;

            // NOTE: ObjファイルのY軸反転処理は、uvTransformで行うため削除
            // texcoord.y = 1.0f - texcoord.y;

            texcoords.push_back(texcoord);
        }
        else if (identifier == "vn") {
            Vector3 normal;
            s >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
        else if (identifier == "mtllib") {
            std::string materialFilename;
            s >> materialFilename;
            modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
        }
        else if (identifier == "f") {
            std::string faceIndex;
            for (int i = 0; i < 3; ++i) {
                s >> faceIndex;
                std::istringstream faceStream(faceIndex);
                std::string segment;

                uint32_t vertexIndex, texcoordIndex, normalIndex;

                std::getline(faceStream, segment, '/');
                vertexIndex = std::stoi(segment);
                std::getline(faceStream, segment, '/');
                texcoordIndex = std::stoi(segment);
                std::getline(faceStream, segment, '/');
                normalIndex = std::stoi(segment);

                modelData.vertices.push_back({
                    {positions[vertexIndex - 1].x, positions[vertexIndex - 1].y, positions[vertexIndex - 1].z, 1.0f},
                    texcoords[texcoordIndex - 1],
                    normals[normalIndex - 1]
                    });
            }
        }
    }
    return modelData;
}

// --- SetModelのオーバーロードを実装 ---
void Object3d::SetModel(const std::string& filePath)
{
    // ModelManagerからファイルを検索してセット
    model = ModelManager::GetInstance()->FindModel(filePath);
}


void Model::Initialize(ModelCommon* modelCommon, const std::string& directoryPath, const std::string& filename)
{
    // ModelCommonのポインタを引数からメンバ変数に記録する
    this->modelCommon_ = modelCommon;

    // 1. モデル読み込み: 引数で渡されたディレクトリとファイル名を使う
    modelData = LoadObjFile(directoryPath, filename);

    // 2. テクスチャのロードとインデックス取得
    if (!modelData.material.textureFilePath.empty()) {
        TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);

        modelData.material.textureIndex =
            TextureManager::GetInstance()->GetSrvIndexByFilePath(modelData.material.textureFilePath); // ★変更
    }

    // 3. 各種リソースの作成（CreateVertexDataなど）
    CreateVertexData();     // 頂点データ初期化
    CreateMaterialData();   // マテリアル初期化
}



// --- Draw (Object3d.cppから移植。モデル固有の処理のみ) ---
void Model::Draw()
{
    // コマンドリスト取得
    auto commandList = modelCommon_->GetDXCommon()->GetCommandList();
    auto textureManager = TextureManager::GetInstance();

    // 1. VertexBufferViewを設定
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

    // 2. マテリアルCBufferの場所を設定 (RootParameter 0)
    commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

    // NOTE: RootParameter 1 (変換行列) はObject3d::Draw()側で設定

    // 3. SRVのDescriptorTableの先頭を設定 (RootParameter 2)
    commandList->SetGraphicsRootDescriptorTable(
        2, textureManager->GetSrvHandleGPUByFilePath(modelData.material.textureFilePath) // ★変更: ファイルパスでアクセス
    );

    // NOTE: RootParameter 3 (Light) はObject3d::Draw()側で設定

    // 4. 描画! (DrawCall)
    commandList->DrawInstanced((UINT)modelData.vertices.size(), 1, 0, 0);
}

// --- CreateVertexData (Object3d.cppから移植) ---
void Model::CreateVertexData()
{
    auto dxCommon = modelCommon_->GetDXCommon();
    const size_t vertexCount = modelData.vertices.size();

    vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * vertexCount);

    vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * vertexCount);
    vertexBufferView.StrideInBytes = sizeof(VertexData);

    vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    std::copy(modelData.vertices.begin(), modelData.vertices.end(), vertexData);
}

// --- CreateMaterialData (Object3d.cppから移植) ---
void Model::CreateMaterialData()
{
    auto dxCommon = modelCommon_->GetDXCommon();

    materialResource = dxCommon->CreateBufferResource(sizeof(Material));
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

    materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData->enableLighting = false;

    // UV反転行列 (Object3d.cppで修正した最終形)
    Matrix4x4 scaleMatrix = MakeScaleMatrix({ -1.0f, -1.0f, 1.0f });
    Matrix4x4 translateMatrix = MakeTranslateMatrix({ 1.0f, 1.0f, 0.0f });
    materialData->uvTransform = Multiply(scaleMatrix, translateMatrix);
}