#include "Zombie.h"
#include <Level/FloorLevel.h>
#include <Renderer/Renderer.h>
#include <cstdlib>

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

void Zombie::Draw()
{
	std::shared_ptr<FloorLevel> floor =
		Cast<FloorLevel>(GetOwner());

	// F1 키를 누르면 마지막으로 계산한 A* 경로가 표시 됨.
	if (floor && floor->IsDebugMode())
	{
		for (const Vector2& pathPosition : latestPath)
		{
			Renderer::Get().Submit(
				"+", pathPosition, Color::Cyan, 2);
		}
	}

	// I : Player 발견 못함.
	const char* stateText = "I";

	if (state == ZombieState::Chase)
	{
		// C : 추격중.
		stateText = "C";
	}
	else if (state == ZombieState::Search)
	{	// S : 마지막으로 본 위치를 조사 중.
		stateText = "S";
	}

	Renderer::Get().Submit(
		stateText, position, Color::Yellow, sortingOrder);

	return;

	Actor::Draw();
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

	Vector2 playerPosition = floor->GetPlayerPosition();

	int distance =
		std::abs(position.x - playerPosition.x)
		+ std::abs(position.y - playerPosition.y);

	if (distance <= detectionRange
		// 거리 안에 있어도 중간에 벽이 있으면 targetPosition을 갱신하지 않음.
		&& floor->HasLineOfSight(position, playerPosition))
	{
		state = ZombieState::Chase;
		targetPosition = playerPosition;
	}
	else if (state == ZombieState::Chase)
	{
		state = ZombieState::Search;
	}

	if (state == ZombieState::Idle)
	{
		latestPath.clear();
		return;
	}

	latestPath = pathFinder.FindPath(
		position,
		targetPosition,
		*floor);

	// 경로가 두 칸보다 짧으면 이미 도착햇거나 길을 찾지 못한 경우라 이동하지 못함.
	if (latestPath.size() < 2)
	{
		return;
	}

	// 경로가 두 칸이면 현재 좀비 칸 -> Player 칸이므로 서로 붙어 있다는 뜻.
	// Zombie는 Player 칸으로 겹쳐 들어가지 않고 그 자리에서 HP를 10 감소시킴.

	if (state == ZombieState::Chase
		&& latestPath.size() == 2)
	{
		floor->DamagePlayer(10);
		latestPath.clear();
		return;
	}

	// 경로를 계산하고 실제 이동 직전에 그 칸이 비어있는지 확인.
	// 다른 좀비가 있으면 이번 이동을 건너뛰고 0.5초 뒤 다시 판단.
	if (floor->HasZombieAt(latestPath[1]))
	{
		return;
	}

	// path[0]은 좀비의 현재 칸. path[1]은 다음 칸.
	SetPosition(latestPath[1]);
	latestPath.erase(latestPath.begin());
	
	// 마지막으로 기억한 위치에 도착하면 Search를 끝나게 함.
	if (state == ZombieState::Search
		&& position == targetPosition)
	{
		state = ZombieState::Idle;
	}

}

// 이 Zombie에게 피해를 줘라 요청하는 함수(public).
void Zombie::TakeDamage(int amount)
{
	// health는 Zombie 내부 데이터(private).
	health -= amount;

	if (health <= 0)
	{
		health = 0;
		// Destroy()가 호출되면 즉시 delete하지 않고, Level의 안전한 제거 과정에서 사라짐.
		Destroy();
	}
}