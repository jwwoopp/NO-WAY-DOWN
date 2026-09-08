#pragma once

#include <Actor/Actor.h>
#include <PathFinding/AStar.h>

class Zombie : public Craft::Actor
{
	TYPE_DECLARATIONS(Zombie, Actor)

public:
	Zombie(const Craft::Vector2& position);

	virtual void Tick(float deltaTime) override;

private:
	// 일정 시간이 지나면 플레이어까지 경로를 구하고 한 칸 이동.
	void ProcessChase(float deltaTime);

	// Zombie의 A* 계산 역할.
	AStar pathFinder;

	float moveInterval = 0.5f;
	float moveTimer = 0.0f;
};