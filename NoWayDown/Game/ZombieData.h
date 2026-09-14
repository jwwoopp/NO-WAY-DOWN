#pragma once

struct ZombieData
{
	int health = 100;
	// 처음 추격을 시작한 층.
	int originFloor = 1;
	// 도착할 층.
	int destinationFloor = 2;
	// 등장 지연 3초.
	float arrivalDelay = 3.0f;
};