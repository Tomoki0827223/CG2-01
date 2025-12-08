#include "Particle.h"
#include "Vector3.h"
#include <random>

// 乱数エンジンをここで初期化し、グローバル（ファイル内静的）で管理します。
std::random_device seedGenerator;
std::mt19937 randomEngine(seedGenerator());

Particle MakeNewParticle()
{
	// main.cppから移動した乱数の設定
	std::uniform_real_distribution<float> distribution(-0.2f, 0.2f);
	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
	std::uniform_real_distribution<float> destTime(1.0f, 8.0f);

	Particle particle;

	// main.cppから移動した初期化ロジック
	particle.transform.scale = { 1.0f, 1.0f, 1.0f };
	particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
	particle.transform.translate = { distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };
	particle.velocity = { distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };
	particle.color = { distColor(randomEngine), distColor(randomEngine), distColor(randomEngine), 1.0f };
	particle.lifeTime = destTime(randomEngine);
	particle.currentTime = 0.0f;

	return particle;
}