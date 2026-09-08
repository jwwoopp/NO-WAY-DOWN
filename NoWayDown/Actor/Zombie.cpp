#include "Zombie.h"
#include <Level/FloorLevel.h>

using namespace Craft;

Zombie::Zombie(const Vector2& position)
	: Actor("Z", position, Color::Red)
{
	// 바닥보다 위에 그리되 Player의 5보다 낮게 둠.
	sortingOrder = 4;
}

// 매 프레임 부모 Actor의 기본 처리 후 추격 시간 계산함.
void Zombie::Tick(float deltaTime)
{
	Actor::Tick(deltaTime);
	ProcessChase(deltaTime);
}

void Zombie::ProcessChase(float deltaTime)
{
	moveTimer += deltaTime;

	if (moveTimer < moveInterval)
	{
		return;
	}

	moveTimer = 0.0f;

	std::shared_ptr<FloorLevel> floor =
		Cast<FloorLevel>(GetOwner());

	if (!floor)
	{
		return;
	}

	std::vector<Vector2> path = pathFinder.FindPath(
		position,
		floor->GetPlayerPosition(),
		*floor);

	// 경로가 두 칸보다 짧으면 이미 도착햇거나 길을 찾지 못한 경우라 이동하지 못함.
	if (path.size() < 2)
	{
		return;
	}

	// path[0]은 좀비의 현재 칸. path[1]은 다음 칸.
	SetPosition(path[1]);
}