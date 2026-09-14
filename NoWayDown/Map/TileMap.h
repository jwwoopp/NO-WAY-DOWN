#pragma once

#include <Map/TileType.h>
#include <vector>

// 어느 칸에 어떤 지형이 있는가를 담당함.
// Actor를 상속하지 않음 : 움직이거나 매 프레임 행동하는 객체가 아님.
// 지형을 저장하고 조회하는 클래스.
class TileMap
{
public:
	// 맵의 가로·세로 크기와 지형 목록을 받아 저장하는 함수
	void SetData(int newWidth, int newHeight,
		std::vector<TileType> newTiles);
	TileType GetTile(int x, int y) const;

	// 해당 칸의 지형을 바꿈.
	void SetTile(int x, int y, TileType newTile);

	int GetWidth() const { return width; }
	int GetHeight() const { return height; }

private:
	std::vector<TileType> tiles;
	int width = 0;
	int height = 0;
};