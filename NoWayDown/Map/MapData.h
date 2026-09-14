#pragma once

#include <Map/TileType.h>
#include <Math/Vector2.h>
#include <vector>

struct MapData
{
	int width = 0;
	int height = 0;
	std::vector<TileType> tiles;

	Craft::Vector2 playerStart = Craft::Vector2::Zero;
	std::vector<Craft::Vector2> zombieStarts;
	std::vector<Craft::Vector2> medicinePositions;
};