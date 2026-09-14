#pragma once

#include <Actor/Actor.h>
#include <PathFinding/AStar.h>
#include <Game/ZombieData.h>

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
	Zombie(const Craft::Vector2& position, const ZombieData& data);

	virtual void Tick(float deltaTime) override;
	virtual void Draw() override;

	void TakeDamage(int amount);
	void HearNoise(const Craft::Vector2& noisePosition);
	// 다음 층에 넘길 현재 체력을 읽음.
	int GetHealth() const { return health; }
	int GetOriginFloor() const { return originFloor; }

	// 화면에 그릴 위치. 실제 타일 좌표를 부드럽게 따라감.
	// 0.5초마다 한 칸씩 순간이동하면 뚝뚝 끊겨 보여서 그리기용 좌표를 따로 둠.
	float GetVisualX() const { return visualX; }
	float GetVisualY() const { return visualY; }
	int GetBobOffset() const;

	// 살아 있고, 추격이나 수색 중이며 관심 시간이 남은 좀비인지 확인.
	// 추격 후보를 고르는 조건.
	// 단, 계단까지 실제로 도달 가능한지와 추격 층 제한은 별도로 판단.
	bool CanFollowToNextFloor() const
	{
		return IsActive()
			&& state != ZombieState::Idle
			&& interestTimer > 0.0f;
	}

private:
	struct Setting
	{
		float detectionRange = 8.0f;
		float moveInterval = 0.5f;
		float interestDuration = 8.0f;
	
		float doorAttackInterval = 1.0f;
		int doorHitsBeforeRest = 3;
		float doorRestDuration = 2.0f;
	};

	Setting setting;

	// 좀비 내부 행동을 결정하는 데이터.
	// state - 현재 행동 상태.
	ZombieState state = ZombieState::Idle;
	bool canSeePlayer = false;

	// targetPosition - 마지막으로 알아낸 목표 위치.
	Craft::Vector2 targetPosition = Craft::Vector2::Zero;
	// Used by the directional sprite sheet; movement and rendering share this.
	// +Y is the side toward the camera, so an idle zombie starts front-facing.
	Craft::Vector2 facingDirection = Craft::Vector2(0, 1);
	int health = 100;
	int originFloor = 0;

	void DetectPlayer();
	// 일정 시간이 지나면 플레이어까지 경로를 구하고 한 칸 이동.
	void Chase(float deltaTime);
	void AttackDoor(const Craft::Vector2& doorPosition);

	// Zombie의 A* 계산 역할.
	AStar pathFinder;

	std::vector<Craft::Vector2> latestPath;

	float moveTimer = 0.0f;

	// 그리기용 좌표. 실제 좌표를 천천히 따라와서 걷는 것처럼 보임.
	void UpdateVisualPosition(float deltaTime);

	float visualX = 0.0f;
	float visualY = 0.0f;
	bool visualReady = false;
	// 이동 간격 0.5초에 맞춰 느리게 잡아야 미끄러지듯 보임.
	float visualFollowSpeed = 8.0f;
	float bobTimer = 0.0f;
	
	// 다음 문 타격까지 남은 시간.
	float doorAttackTimer = 0.0f;
	// 쉬어야 하는 남은 시간.
	float doorRestTimer = 0.0f;
	// 이번에 연속으로 두드린 횟수.
	int consecutiveDoorHits = 0;

	// 새 단서 없이 수색을 유지할 남은 시간.
	float interestTimer = 0.0f;
};
