#pragma once

#include "AStarNode.h"
#include <vector>

// FloorLevel 클래스 전방선언.
// 실제 함수에 필요한 헤더는 AStar.cpp에서 구현.
class FloorLevel;

struct AStarStats
{
	int searchCount = 0;
	int failedCount = 0;
	int expandedNodes = 0;
	double totalMicroseconds = 0.0;
	double maxMicroseconds = 0.0;
};

class AStar
{
public:
	~AStar();

	static const AStarStats& GetStats()
	{
		return stats;
	}

	static void ResetStats()
	{
		stats = AStarStats{};
	}

	// FindPath는 시작 좌표, 목적지, 현재 층을 받아 이동할 좌표 목록을 반환.
	std::vector<Craft::Vector2> FindPath(
		const Craft::Vector2& start,
		const Craft::Vector2& goal,
		// Floor은 const 참조로 받아 맵을 복사하지 않고 읽기만 함.
		const FloorLevel& floor,
		// canBreakDoors - 문을 부수는 경로도 고려할 것인가라는 옵션.
		bool canBreakDoors = false);
private:
	struct Setting
	{
		float floorStepCost = 1.0f;
		float closedDoorStepCost = 10.0f;
	};

	Setting setting;

	inline static AStarStats stats;
	
	std::vector<Craft::Vector2> FindPathInternal(
		const Craft::Vector2& start,
		const Craft::Vector2& goal,
		const FloorLevel& floor,
		bool canBreakDoors);

	// 생성한 Node와 탐색 목록을 정리하고, 소멸자는 AStar가 사라질 때 Clear을 호출하도록 구현.
	void Clear();

	// Node를 만들고, allocatedNodes에 기록.
	AStarNode* CreateNode
	(
		const Craft::Vector2& position,
		AStarNode* parent = nullptr
	);

	float CalculateHeuristic(
		const Craft::Vector2& current,
		const Craft::Vector2& goal) const;

	std::vector<Craft::Vector2> ConstructPath(AStarNode* destination);

	AStarNode* FindOpenNode(const Craft::Vector2& position) const;

	bool IsInClosedList(const Craft::Vector2& position) const;


	// openList - 앞으로 조사할 후보.
	std::vector<AStarNode*> openList;
	// closedList - 주변 칸 검사를 마친 Node를 기록.
	// 두 목록은 Node의 주소만 보관.
	std::vector<AStarNode*> closedList;

	// 생성한 Node를 빠짐없이 정리하기 위한 목록.
	std::vector<AStarNode*> allocatedNodes;
};