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

void ModelManager::Load(const std::string& filePath)
{
    // 既存のLoadModel関数を呼び出す
    LoadModel(filePath);
}

Model* ModelManager::GetModel(uint32_t modelIndex) const
{
    // インデックスが範囲外でないかチェック
    if (modelIndex >= models.size()) {
        // 範囲外ならnullptrを返す
        return nullptr;
    }

    // イテレータを先頭に設定
    // models が std::map<string, unique_ptr<Model>> であることを前提とします
    auto it = models.cbegin(); // const iterator を取得

    // modelIndexの回数だけイテレータを進める (手動ループを使用)
    // ※ C++11以降なら std::advance(it, modelIndex); が使えますが、互換性のためループで記述します。
    for (uint32_t i = 0; i < modelIndex; ++i) {
        ++it;
    }

    // イテレータが指す要素 (it->second は unique_ptr<Model>) の生ポインタを取得
    return it->second.get();
}