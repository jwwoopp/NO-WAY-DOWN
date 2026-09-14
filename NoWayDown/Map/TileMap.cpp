#include "TileMap.h"
#include <utility>

// TileMap이 가진 지형 조회.
TileType TileMap::GetTile(int x, int y) const
{
	// 맵 밖의 경우, 벽으로 반환해서 배열 범위 벗어나지 않게 함.
	if (x < 0 || x >= width || y < 0 || y >= height)
	{
		return TileType::Wall;
	}
	return tiles[(y * width) + x];
}

void TileMap::SetData(int newWidth, int newHeight,
	std::vector<TileType> newTiles)
{
	width = newWidth;
	height = newHeight;
	// 지형 원소들을 하나씩 복사하는 대신, 전달받은 vector의 저장 공간을 멤버 tiles로 넘겨 받는 역할.
	tiles = std::move(newTiles);
}

void TileMap::SetTile(int x, int y, TileType newTile)
{
	// 맵 밖 좌표는 변경하지 않고 바로 돌아감.
	if (x < 0 || x >= width || y < 0 || y >= height)
	{
		return;
	}

	tiles[(y * width) + x] = newTile;
}