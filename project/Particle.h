#pragma once

#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include <cstdint>
#include <random>

// 最大パーティクル数 (main.cppから抽出)
const uint32_t kNumMaxParticleInstance = 50;

// Transform構造体 (main.cppから抽出)
struct TransformVector3
{
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

// GPUに送るパーティクルのインスタンシングデータ (main.cppから抽出)
struct ParticleForGPU
{
	Matrix4x4 WVP;
	Matrix4x4 world;
	Vector4 color;
};

// パーティクルのCPU側データ (main.cppから抽出)
struct Particle
{
	TransformVector3 transform;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};

// 新しいパーティクルを生成する関数
// 乱数エンジンはParticle.cpp内で管理します。
Particle MakeNewParticle();