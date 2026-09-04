#pragma once

#include <Level/Level.h>
#include <Math/Vector2.h>
#include <Map/TileType.h>
#include <string>
#include <vector>

// NO WAY DOWN의 한 층을 담당하는 레벨.
class FloorLevel : public Craft::Level
{
	TYPE_DECLARATIONS(FloorLevel, Level)

public:
	// 해당 좌표로 이동할 수 있는지 판정.
	// Plyaer가 이동 전에 물어봐야 하니까 public.
	bool IsWalkable(const Craft::Vector2& position) const;

	// 해당 좌표의 타일 종류를 반환.
	TileType GetTile(int x, int y) const;

	int GetWidth() const { return width; }
	int GetHeight() const { return height; }

private:
	// OnInitialized와 Draw는 Engine이 부르기 때문에 private.
	virtual void OnInitialized() override;
	virtual void Draw() override;

	// 맵 파일을 읽어 tiles를 채우는 함수.
	void LoadMap(const std::string& filename);

private:
	// 층의 정적 지형. 1차원 배열을 (y * width) + x로 접근.
	std::vector<TileType> tiles;

	int width = 0;
	int height = 0;

	// 맵 파일에서 찾은 플레이어 시작 좌표.
	Craft::Vector2 playerStart;
};