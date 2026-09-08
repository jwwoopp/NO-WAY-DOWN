#pragma once

#include <Actor/Actor.h>
#include <PathFinding/AStar.h>

enum class ZombieState
{
	// 플레이어를 모르는 상태, 추격하는 상태, 마지막 위치 조사하는 상태로 나눔.
	Idle,
	Chase,
	Search
};

class Zombie : public Craft::Actor
{
	TYPE_DECLARATIONS(Zombie, Actor)

public:
	Zombie(const Craft::Vector2& position);

	virtual void Tick(float deltaTime) override;
	virtual void Draw() override;

	void TakeDamage(int amount);

private:
	// 좀비 내부 행동을 결정하는 데이터.
	// state - 현재 행동 상태.
	ZombieState state = ZombieState::Idle;

	// targetPosition - 마지막으로 알아낸 목표 위치.
	Craft::Vector2 targetPosition = Craft::Vector2::Zero;

	// detectionRange - 감지 거리.
	float detectionRange = 8.0f;
	int health = 100;

	// 일정 시간이 지나면 플레이어까지 경로를 구하고 한 칸 이동.
	void ProcessChase(float deltaTime);

	// Zombie의 A* 계산 역할.
	AStar pathFinder;

	std::vector<Craft::Vector2> latestPath;

	float moveInterval = 0.5f;
	float moveTimer = 0.0f;
};