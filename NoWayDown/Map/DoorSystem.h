#pragma once

#include <Map/DoorData.h>
#include <vector>

class TileMap;

class DoorSystem
{
public:
	// 맵의 문 위치를 수집.
	void Initialize(const TileMap& tileMap);
	// 문 피격 횟수를 올려 10회에 도달하면 문을 파괴하도록 구현.
	bool HitDoor(TileMap& tileMap, const Craft::Vector2& position);

private:
	struct Setting
	{
		int hitsToBreak = 10;
	};

	Setting setting;

	std::vector<DoorData> doors;
};