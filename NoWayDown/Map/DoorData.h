#pragma once

#include <Math/Vector2.h>

struct DoorData
{
	Craft::Vector2 position = Craft::Vector2::Zero;
	// 문이 지금까지 맞은 횟수.
	// 문을 열고 닫아도 초기화할 수 없음.
	int hitCount = 0;
};