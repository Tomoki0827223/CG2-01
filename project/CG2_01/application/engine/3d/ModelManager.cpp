#include "ModelManager.h"
#include "Logger.h"
#include <cassert>
#include <utility>


// 静的メンバの初期化
ModelManager* ModelManager::instance = nullptr;

// --- GetInstance ---
ModelManager* ModelManager::GetInstance()
{
    if (instance == nullptr) {
        instance = new ModelManager();
    }
    return instance;
}

// --- Initialize ---
void ModelManager::Initialize(DirectXCommon* dxCommon)
{
    // モデル共通部のインスタンス生成と初期化をModelManagerに移行
    modelCommon = new ModelCommon();
    modelCommon->Initialize(dxCommon);
}

// --- Finalize ---
void ModelManager::Finalize()
{
    // モデル共通部を解放
    delete modelCommon;
    modelCommon = nullptr;

    // ModelManagerのインスタンスを解放
    delete instance;
    instance = nullptr;

    // std::mapのunique_ptrは自動で解放される
    // models.clear(); // 呼び出す必要はないが、明示的にクリアしてもよい
}

// --- LoadModel ---
void ModelManager::LoadModel(const std::string& filePath)
{
    // 読み込み済みモデルを検索
    if (models.contains(filePath)) {
        // 読み込み済みなら早期return
        return;
    }

    // モデルの生成とファイル読み込み、初期化
    // 1. Modelインスタンスを生成
    std::unique_ptr<Model> model = std::make_unique<Model>();

    // 2. モデルを初期化し、Objファイルを読み込む ("resources"ディレクトリは固定)
    // NOTE: Model::InitializeにはdirectoryPathとfilenameを渡すように修正済みとする
    model->Initialize(modelCommon, "resources", filePath);

    // 3. モデルをmapコンテナに格納 (所有権をstd::moveで移動)
    models.insert(std::make_pair(filePath, std::move(model)));
}

// --- FindModel ---
Model* ModelManager::FindModel(const std::string& filePath)
{
    // 読み込み済みモデルを検索
    if (models.contains(filePath)) {
        // 読み込み済みモデルを戻り値として返す (生ポインタ)
        return models.at(filePath).get();
    }

    // ファイル名一致なし
    return nullptr;
}