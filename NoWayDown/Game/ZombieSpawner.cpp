#include "ZombieSpawner.h"

#include <Level/FloorLevel.h>
#include <Actor/Zombie.h>
#include <Game/RunState.h>
#include <Game/ZombieData.h>
#include <PathFinding/AStar.h>

using namespace Craft;

void ZombieSpawner::SpawnMapZombies(FloorLevel& floor)
{
	for (const Vector2& zombieStart : floor.GetZombieStarts())
	{
		floor.SpawnActor<Zombie>(zombieStart);
	}
}

void ZombieSpawner::Update(
	float deltaTime, FloorLevel& floor, RunState& runState)
{
	// 이 층으로 오고 있는 좀비들의 남은 시간을 줄임.
	for (ZombieData& data : runState.incomingZombies)
	{
		if (data.destinationFloor == runState.currentFloor)
		{
			data.arrivalDelay -= deltaTime;
		}
	}

	// 도착할 때가 된 좀비를 한 마리만 만들고 끝냄.
	// erase 직후 바로 빠져나오므로 반복자가 무효화될 일이 없음.
	for (auto iter = runState.incomingZombies.begin();
		iter != runState.incomingZombies.end(); ++iter)
	{
		if (iter->destinationFloor != runState.currentFloor
			|| iter->arrivalDelay > 0.0f)
		{
			continue;
		}

		Vector2 spawnPosition;

		// 계단 근처에서 가장 가까운 빈 칸을 찾음.
		// 빈 칸이 없으면 이번 프레임은 넘기고 다음에 다시 시도함.
		if (!floor.FindSpawnPosition(floor.GetPlayerStart(), spawnPosition))
		{
			return;
		}

		std::shared_ptr<Zombie> zombie =
			floor.SpawnActor<Zombie>(spawnPosition, *iter);

		// 계단이 아니라 플레이어의 현재 위치를 목표로 줌.
		// 계단을 목표로 주면 도착하자마자 목표에 닿았다고 판단해 추격을 포기함.
		zombie->HearNoise(floor.GetPlayerPosition());

		runState.incomingZombies.erase(iter);
		return;
	}
}

void ZombieSpawner::SaveFollowingZombies(
	FloorLevel& floor, RunState& runState)
{
	AStar pathFinder;

	for (const std::shared_ptr<Zombie>& zombie : floor.GetActiveZombies())
	{
		if (!zombie->CanFollowToNextFloor())
		{
			continue;
		}

		// 플레이어까지 실제로 갈 수 있는 좀비만 따라옴.
		// 벽 너머에서 헤매던 좀비가 계단을 타고 나타나면 이상함.
		auto path = pathFinder.FindPath(
			zombie->GetPosition(), floor.GetPlayerPosition(), floor, false);

		if (path.empty())
		{
			continue;
		}

		ZombieData data;
		data.health = zombie->GetHealth();
		data.originFloor = zombie->GetOriginFloor();

		// 이 층에서 처음 추격을 시작한 좀비는 출발 층이 비어 있음.
		if (data.originFloor == 0)
		{
			data.originFloor = runState.currentFloor;
		}

		data.destinationFloor = runState.currentFloor + 1;

		// 처음 추격을 시작한 층에서 maxChaseFloors만큼 멀어지면 추격을 포기함.
		// 이게 없으면 좀비 한 마리가 1층부터 12층까지 끝없이 따라올 수 있음.
		if (data.destinationFloor - data.originFloor > maxChaseFloors)
		{
			continue;
		}

		runState.incomingZombies.emplace_back(data);
	}
}
