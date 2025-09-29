#include "Object3d.h"
#include "Object3dCommon.h" // Object3dCommonの関数呼び出しのためにインクルード
#include "DirectXCommon.h" // リソース作成のために必要
#include <fstream>
#include <sstream>
#include <cassert>
#include "affine.h"
#include "TextureManager.h"

// --- LoadMaterialTemplateFileの定義 (Objファイル読み込みに必要) ---
Object3d::MaterialData Object3d::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
    Object3d::MaterialData materialData;
    std::string line;
    std::string fullPath = directoryPath + "/" + filename;
    std::ifstream file(fullPath);

    // ファイルが開けなかった場合はアサート
    assert(file.is_open());

    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;

        if (identifier == "map_Kd") {
            // テクスチャファイル名
            std::string textureFileName;
            s >> textureFileName;
            materialData.textureFilePath = directoryPath + "/" + textureFileName;
        }
    }
    return materialData;
}

// --- LoadObjFileの定義 ---
Object3d::ModelData Object3d::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
    Object3d::ModelData modelData;
    std::vector<Vector3> positions; // 頂点座標
    std::vector<Vector3> normals;   // 法線ベクトル
    std::vector<Vector2> texcoords; // UV座標
    std::string line;
    std::string fullPath = directoryPath + "/" + filename;
    std::ifstream file(fullPath);

    // ファイルが開けなかった場合はアサート
    assert(file.is_open());

    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;

        // 頂点座標
        if (identifier == "v") {
            Vector3 position;
            s >> position.x >> position.y >> position.z;
            positions.push_back(position);
        }
        // UV座標
        else if (identifier == "vt") {
            Vector2 texcoord;
            s >> texcoord.x >> texcoord.y;
            // UV座標はY方向が反転しているため補正
            texcoord.y = 1.0f - texcoord.y;
            texcoords.push_back(texcoord);
        }
        // 法線ベクトル
        else if (identifier == "vn") {
            Vector3 normal;
            s >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
        // マテリアルファイル名の読み込み
        else if (identifier == "mtllib") {
            std::string materialFilename;
            s >> materialFilename;
            modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
        }
        // 面情報 (頂点インデックス)
        else if (identifier == "f") {
            std::string faceIndex;
            for (int i = 0; i < 3; ++i) {
                s >> faceIndex;

                // v/vt/vn の形式を解析
                std::istringstream faceStream(faceIndex);
                std::string segment;

                uint32_t vertexIndex, texcoordIndex, normalIndex;

                // 座標インデックス
                std::getline(faceStream, segment, '/');
                vertexIndex = std::stoi(segment);

                // UVインデックス
                std::getline(faceStream, segment, '/');
                texcoordIndex = std::stoi(segment);

                // 法線インデックス
                std::getline(faceStream, segment, '/');
                normalIndex = std::stoi(segment);

                // インデックスは1始まりなので-1して格納
                modelData.vertices.push_back({
                    {positions[vertexIndex - 1].x, positions[vertexIndex - 1].y, positions[vertexIndex - 1].z, 1.0f},
                    texcoords[texcoordIndex - 1],
                    normals[normalIndex - 1]
                    });
            }
        }
    }

    // Objファイルの描画は通常TRIANGLELISTですが、ここでは読み込んだ頂点をそのままリストとして使用します。
    // IndexBufferを使わないため、頂点リストのまま格納します。
    return modelData;
}

void Object3d::Initialize(Object3dCommon* object3dCommon)
{
    this->object3dCommon = object3dCommon;

    // --- 1. モデル読み込み (テクスチャパスを含む) ---
    // Objファイルとmtlファイルを読み込み、textureFilePathがmodelDataに格納される
    modelData = LoadObjFile("resources", "plane.obj");

    // --- 2. テクスチャのロードとインデックス取得 (LoadObjFileの後に実行) ---
    // LoadObjFile内で LoadTexture が呼ばれる場合はこのブロックは不要ですが、
    // 安全のため、Obj読み込みが完了した後に明示的に実行します。
    if (!modelData.material.textureFilePath.empty()) {
        TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);

        modelData.material.textureIndex =
            TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData.material.textureFilePath);
    }


    // --- [🚨 追加箇所 4: Transformの初期値設定 🚨] ---
    // Object Transformの初期設定
    transform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

    // Camera Transformの初期設定 (mainから移植した処理)
    cameraTransform = { {1.0f, 1.0f, 1.0f}, {0.3f, 0.0f, 0.0f}, {0.0f, 4.0f, -10.0f} };
    // ----------------------------------------------------


    // 2. 各種リソースの作成（CreateVertexDataなどはInitialize内で呼び出し）
    CreateVertexData();
    CreateMaterialData();
    CreateTransformationMatrixData();
    CreateDirectionalLightData();
}


void Object3d::Update()
{
    // 修正案: 1/10の速度にする
    transform.rotate.y += 0.001f;

    // --- [🚨 追加箇所 5: World-View-Projection行列の計算 🚨] ---
    // 1. TransformからWorldMatrixを作る
    Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

    // 2. cameraTransformからCameraMatrixを作る
    Matrix4x4 cameraMatrix = MakeAffineMatrix(
        cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate
    );

    // 3. cameraMatrixからViewMatrixを作る
    Matrix4x4 viewMatrix = Inverse(cameraMatrix);

    // 4. ProjectionMatrixを作る (ここでは透視投影)
    Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(
        0.45f, (float)WinApp::kClientWidth / (float)WinApp::kClientHeight, 0.1f, 100.0f
    );
    // ※ WinApp::kClientWidth/kClientHeight にアクセスできる必要があります。

    // 5. 結果をTransformationMatrixDataに書き込む
    transformationMatrixData->WVP = Multiply(
        worldMatrix, Multiply(viewMatrix, projectionMatrix)
    );
    transformationMatrixData->World = worldMatrix;
    // ----------------------------------------------------------------------
}

void Object3d::Draw()
{
    // コマンドリスト取得（Object3dCommon経由）
    auto commandList = object3dCommon->GetDXCommon()->GetCommandList();

    // 1. VertexBufferViewを設定
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

    // 2. マテリアルCBufferの場所を設定 (RootParameter 0)
    commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

    // 3. 座標変換行列CBufferの場所を設定 (RootParameter 1)
    commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

    // 4. SRVのDescriptorTableの先頭を設定 (RootParameter 2)
    commandList->SetGraphicsRootDescriptorTable(
        2, TextureManager::GetInstance()->GetSrvHandleGPU(modelData.material.textureIndex)
    );

    // 5. 平行光源CBufferの場所を設定 (RootParameter 3)
    commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

    // 6. 描画! (DrawCall)
    // IndexBufferは使わないため、DrawInstancedを呼び出す
    commandList->DrawInstanced((UINT)modelData.vertices.size(), 1, 0, 0);
}

void Object3d::CreateVertexData()
{
    // 頂点データの作成 (Object3d::Initializeから移植)

    // デバイス取得
    auto dxCommon = object3dCommon->GetDXCommon();

    // 頂点数 (Objファイルから読み込んだ頂点の総数)
    const size_t vertexCount = modelData.vertices.size();

    // VertexResourceを作る
    vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * vertexCount);

    // VertexBufferViewを作成する（値を設定するだけ）
    vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * vertexCount);
    vertexBufferView.StrideInBytes = sizeof(VertexData);

    // VertexResourceにデータを書き込むためのアドレスを取得してvertexDataに割り当てる
    vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

    // 読み込んだ頂点データをConstantBufferにコピー
    std::copy(modelData.vertices.begin(), modelData.vertices.end(), vertexData);
}

void Object3d::CreateMaterialData()
{
    auto dxCommon = object3dCommon->GetDXCommon();

    // マテリアルリソースを作る
    materialResource = dxCommon->CreateBufferResource(sizeof(Material));

    // データ書き込みアドレスを取得し、materialDataに割り当てる
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

    // マテリアルデータの初期値を書き込む
    materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData->enableLighting = false;
    materialData->uvTransform = MakeIdentity4x4();
}

void Object3d::CreateTransformationMatrixData()
{
    auto dxCommon = object3dCommon->GetDXCommon();

    // 座標変換行列リソースを作る
    transformationMatrixResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));

    // データ書き込みアドレスを取得し、transformationMatrixDataに割り当てる
    transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));

    // 単位行列を書き込んでおく
    transformationMatrixData->WVP = MakeIdentity4x4();
    transformationMatrixData->World = MakeIdentity4x4();
}

void Object3d::CreateDirectionalLightData()
{
    auto dxCommon = object3dCommon->GetDXCommon();

    // 平行光源リソースを作る
    directionalLightResource = dxCommon->CreateBufferResource(sizeof(DirectionalLight));

    // データ書き込みアドレスを取得し、directionalLightDataに割り当てる
    directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));

    // デフォルト値を書き込んでおく (自分で考える箇所)
    // 例: 真上から当たる白い光
    directionalLightData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    directionalLightData->direction = { 0.0f, -1.0f, 0.0f }; // Y軸下向き（真上から）
    directionalLightData->intensity = 1.0f;
}