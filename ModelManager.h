#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <string>
#include <map>
#include <memory> // std::unique_ptr, std::make_unique のために必要
#include "Model.h"
#include "ModelCommon.h"

class ModelManager
{
public:
 
    // シングルトンインスタンスの取得
    static ModelManager* GetInstance();
    // 初期化 (DirectXCommonを引数に取る)
    void Initialize(DirectXCommon* dxCommon);
    // 終了
    void Finalize();

    // モデルファイルの読み込み
    void LoadModel(const std::string& filePath);

    // 格納したモデルデータの取得
    Model* FindModel(const std::string& filePath);

private:
    // プライベートコンストラクタ
    ModelManager() = default;
    // コピー禁止
    ModelManager(const ModelManager&) = delete;
    ModelManager& operator=(const ModelManager&) = delete;
    // privateデストラクタもここではdefaultにしておく

    // シングルトンインスタンス
    static ModelManager* instance;

    // モデル共通部
    ModelCommon* modelCommon = nullptr; // main.cppから所有権を移行

    // モデルデータコンテナ: ファイルパスをキーに、Modelポインタを格納
    std::map<std::string, std::unique_ptr<Model>> models;
};