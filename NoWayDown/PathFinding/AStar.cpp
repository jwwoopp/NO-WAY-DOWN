#include "AStar.h"
#include <Level/FloorLevel.h>
#include <cstdlib>
#include <algorithm>
#include <chrono>

AStarNode* AStar::CreateNode(
	const Craft::Vector2& position,
	AStarNode* parent)
{
	// Node 생성.
	AStarNode* newNode = new AStarNode(position, parent);
	// 나중에 삭제할 수 있도록 주소를 allocatedNodes에 기록한 뒤 반환.
	allocatedNodes.emplace_back(newNode);
	return newNode;
}

void AStar::Clear()
{
	for (AStarNode* node : allocatedNodes)
	{
		// 실제 노드를 해제함.
		// 같은 Node를 중복삭제하지 않도록 allocatedNodes에서만 delete.	
		delete node;
	}

	// 목록에 남은 주소들을 지움.
	allocatedNodes.clear();
	openList.clear();
	closedList.clear();
}

float AStar::CalculateHeuristic(
	const Craft::Vector2& current,
	const Craft::Vector2& goal) const
{
	// std::abs는 음수를 양수로 바꾸는 절댓값 함수.
	// 아직 벽을 고려하지 않은 예상 비용.
	// 실제 이동 비용은 이후 g에 기록.
	int diffX = std::abs(current.x - goal.x);
	int diffY = std::abs(current.y - goal.y);

	return static_cast<float>(diffX + diffY);
}

AStar::~AStar()
{
	// 새로운 경로 탐색을 시작할 때도 호출하여 이전 탐색 기록을 정리할 예정.
	Clear();
}

std::vector<Craft::Vector2> AStar::ConstructPath(AStarNode* destination)
{
	std::vector<Craft::Vector2> path;
	AStarNode* current = destination;

	while (current != nullptr)
	{
		// 목적지로부터 parent로 따라가므로 처음에는 '목적지->시작점' 순서로 저장.
		path.emplace_back(current->position);
		current = current->parent;
	}

	// reverse로 뒤집어 좀비가 사용할 '시작점->목적지' 순서로 반환.
	std::reverse(path.begin(), path.end());
	return path;
}

AStarNode* AStar::FindOpenNode(const Craft::Vector2& position) const
{
	// 같은 좌표의 후보를 찾으면 그 Node의 주소를, 없으면 null을 반환.
	// 이미 후보인 칸을 중복 생성하지 않음.
	// 더 짧은 경로를 찾았을 때 기존 Node의 비용과 parent를 갱신하는 데 사용.
	for (AStarNode* node : openList)
	{
		if (node->position == position)
		{
			return node;
		}
	}
	return nullptr;
}

// 해당 칸의 탐색을 이미 마쳤는지만 확인하므로 bool 반환.
bool AStar::IsInClosedList(const Craft::Vector2& position) const
{
	for (AStarNode* node : closedList)
	{
		if (node->position == position)
		{
			return true;
		}
	}

	return false;
}

std::vector<Craft::Vector2> AStar::FindPathInternal(
	const Craft::Vector2& start,
	const Craft::Vector2& goal,
	const FloorLevel& floor,
	bool canBreakDoors)
{
	// 이전 탐색 기록을 지움.
	Clear();

	// 시작점과 목적지가 벽인지 확인.
	bool canReachGoal = floor.IsWalkable(goal)
		|| (canBreakDoors
			&& floor.GetTile(goal.x, goal.y) == TileType::ClosedDoor);

	if (!floor.IsWalkable(start) || !canReachGoal)
	{
		return {};
	}

	AStarNode* startNode = CreateNode(start);
	startNode->gCost = 0.0f;
	startNode->hCost = CalculateHeuristic(start, goal);
	startNode->fCost = startNode->gCost + startNode->hCost;

	openList.emplace_back(startNode);

	const std::vector<Craft::Vector2> directions =
	{
		Craft::Vector2(0, -1),
		Craft::Vector2(0, 1),
		Craft::Vector2(-1, 0),
		Craft::Vector2(1, 0)
	};

	// 1. openList가 빌 때까지 반복.
	while (!openList.empty())
	{
		AStarNode* currentNode = openList[0];

		for (AStarNode* node : openList)
		{
			// F가 가장 낮은 NOde를 고름.
			if (node->fCost < currentNode->fCost
				// 만약 F가 같으면 H가 낮은 쪽을 고름.
				|| (node->fCost == currentNode->fCost
					&& node->hCost < currentNode->hCost))
			{
				currentNode = node;
			}
		}

		// 만약 목적지라면, parent를 따라 경로를 만들어 반환.
		if (currentNode->position == goal)
		{
			return ConstructPath(currentNode);
		}

		auto iterator =
			std::find(openList.begin(), openList.end(), currentNode);

		// 아니면 openList에서 빼고 closedLIst로 옮김.
		openList.erase(iterator);
		closedList.emplace_back(currentNode);

		// 각 방향의 이웃 칸 검사.
		for (const Craft::Vector2& direction : directions)
		{
			Craft::Vector2 nextPosition =
				currentNode->position + direction;

			// 벽이거나 이미 끝낸 칸이면 건너 뜀.
			bool isClosedDoor =
				floor.GetTile(nextPosition.x, nextPosition.y)
				== TileType::ClosedDoor;

			//목적지가 소리가 난 문 자체일 수도 있음.
			if ((!floor.IsWalkable(nextPosition)
				&& !(canBreakDoors && isClosedDoor))
				|| IsInClosedList(nextPosition))
			{
				continue;
			}

			// 일반 바닥은 1, 닫힌 문은 10.
			// 여기서 10은 실제 타격 횟수가 아니라 우회와 문 파괴 경로를 비교하는 가중치.
			float stepCost = isClosedDoor
				? setting.closedDoorStepCost
				: setting.floorStepCost;
			float newGCost = currentNode->gCost + stepCost;
			AStarNode* openNode = FindOpenNode(nextPosition);

			if (openNode != nullptr)
			{
				// 이미 후보라면 더 짧은 gCost를 찾았을 때만 경로를 수정.
				if (newGCost < openNode->gCost)
				{
					openNode->gCost = newGCost;
					openNode->fCost = newGCost + openNode->hCost;
					openNode->parent = currentNode;
				}

				continue;
			}

			// 처음 발견한 칸은 새 node로 만듦.
			AStarNode* neighborNode =
				CreateNode(nextPosition, currentNode);

			neighborNode->gCost = newGCost;
			neighborNode->hCost =
				CalculateHeuristic(nextPosition, goal);
			neighborNode->fCost =
				neighborNode->gCost + neighborNode->hCost;

			openList.emplace_back(neighborNode);
		}
	}

	return {};
}

std::vector<Craft::Vector2> AStar::FindPath(
	const Craft::Vector2& start,
	const Craft::Vector2& goal,
	const FloorLevel& floor,
	bool canBreakDoors)
{
	const auto begin = std::chrono::steady_clock::now();

	auto path = FindPathInternal(start, goal, floor, canBreakDoors);

	const auto end = std::chrono::steady_clock::now();
	const double elapsed =
		std::chrono::duration<double, std::micro>(end - begin).count();

	++stats.searchCount;
	stats.expandedNodes += static_cast<int>(closedList.size());
	stats.totalMicroseconds += elapsed;

	if (elapsed > stats.maxMicroseconds)
		stats.maxMicroseconds = elapsed;

	if (path.empty())
		++stats.failedCount;

	return path;
}
