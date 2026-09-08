#pragma once

#include <Actor/Actor.h>

// 플레이어 액터.
class Player : public Craft::Actor
{
	TYPE_DECLARATIONS(Player, Actor)

public:
	Player(const Craft::Vector2& position);

	virtual void Tick(float deltaTime) override;
	virtual void Draw() override;
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
	// 게임 시작 시에는 오른쪽을 바라 봄.
	Craft::Vector2 facingDirection = Craft::Vector2(1, 0);

	float attackInterval = 0.4f;
	float attackTimer = 0.0f;
	
	// 공격 위치는 공격 순간의 좌표로 고정.
	// 이후 Player가 움직여도 표시가 따라가지 않음.
	Craft::Vector2 attackEffectPosition = Craft::Vector2::Zero;
	float attackEffectTimer = 0.0f;
	// 명중 여부를 기억.
	bool attackHit = false;
};