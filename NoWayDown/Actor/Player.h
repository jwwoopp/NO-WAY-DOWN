#pragma once

#include <Actor/Actor.h>

// 플레이어 액터.
class Player : public Craft::Actor
{
	TYPE_DECLARATIONS(Player, Actor)

public:
	Player(const Craft::Vector2& position);

	virtual void Tick(float deltaTime) override;

private:
	// 방향키 입력을 받아 이동을 시도하는 함수.
	void ProcessMove(float deltaTime);

private:
	// 한 칸 이동에 걸리는 시간(초). 값이 작을수록 빠름.
	// 설정값.
	float moveInterval = 0.12f;

	// 마지막 이동 이후 흐른 시간.
	// 누적값.
	float moveTimer = 0.0f;
};