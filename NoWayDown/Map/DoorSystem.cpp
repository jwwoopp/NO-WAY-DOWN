#include "DoorSystem.h"
#include <Map/TileMap.h>

void DoorSystem::Initialize(const TileMap& tileMap)
{
	doors.clear();
	
	// 맵을 한 칸씩 확인해서 열린문과 닫힌문만 목록에 등록함.
	for (int y = 0; y < tileMap.GetHeight(); ++y)
	{
		for (int x = 0; x < tileMap.GetWidth(); ++x)
		{
			TileType tile = tileMap.GetTile(x, y);

			if (tile != TileType::ClosedDoor
				&& tile != TileType::OpenDoor)
			{
				continue;
			}
			
			// DoorData의 hitCount는 기본값인 0으로 시작.
			DoorData door;
			door.position = Craft::Vector2(x, y);
			doors.emplace_back(door);
		}
	}
}

// 실제 문 데이터를 수정해서 피격 횟수 누적.
bool DoorSystem::HitDoor(
	TileMap& tileMap, const Craft::Vector2& position)
{
	// 닫힌 문만 피해를 받음.
	if (tileMap.GetTile(position.x, position.y) != TileType::ClosedDoor)
	{
		return false;
	}

	for (DoorData& door : doors)
	{
		if (door.position != position)
		{
			continue;
		}

		++door.hitCount;

		// 열 번째 타격에서 바닥으로 바뀜.
		// 다시 닫을 수 없음.
		if (door.hitCount >= setting.hitsToBreak)
		{
			tileMap.SetTile(position.x, position.y, TileType::Floor);
			return true;
		}

		// 이번엔 파괴되지 않았다.
		return false;
	}

	return false;
}