#include "Sprite.h"

Sprite::~Sprite()
{
	delete sprite;
}

void Sprite::Initialize()
{
	//スプライト共通部分の初期化
	sprite = new Sprite();
	sprite->Initialize();
}
