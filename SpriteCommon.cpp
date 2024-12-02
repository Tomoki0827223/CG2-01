#include "SpriteCommon.h"

SpriteCommon::~SpriteCommon()
{
	delete spriteCommon;
}

void SpriteCommon::Initialize()
{
	//スプライト共通部分の初期化
	spriteCommon = new SpriteCommon();
	spriteCommon->Initialize();


}
