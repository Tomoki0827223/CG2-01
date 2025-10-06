#pragma once

#include <string>
#include <list>
#include <vector>
#include "Particle.h"
#include "Matrix4x4.h"

// パーティクルグループの構造体
struct ParticleGroup {
    // --- マテリアル・リソース関連 ---
    std::string textureFilePath; // マテリアルデータ（テクスチャファイルパス）
    uint32_t textureSrvIndex;    // テクスチャ用SRVインデックス

    // --- インスタンシング関連 ---
    uint32_t instancingSrvIndex; // インスタンシングデータ用SRVインデックス
    void* instancingResource;    // インスタンシングリソース (StructuredBuffer)
    uint32_t instanceCount;      // インスタンス数
    void* instancingDataPtr;     // インスタンシングデータを書き込むためのポインタ
    std::vector<ParticleInstancingData> instancingDatas; // GPUに送るデータ配列

    // --- パーティクルリスト ---
    std::list<Particle> particles; // パーティクルのリスト
};