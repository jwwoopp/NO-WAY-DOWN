#pragma once

#include <Actor/Actor.h>
#include <vector>

// 플레이어 액터.
class Player : public Craft::Actor
{
	TYPE_DECLARATIONS(Player, Actor)

public:
	Player(const Craft::Vector2& position);

	virtual void Tick(float deltaTime) override;
	virtual void Draw() override;

	// 화면에 그릴 위치. 실제 타일 좌표를 부드럽게 따라감.
	// 칸 단위로 순간이동하면 딱딱해 보여서 그리기용 좌표를 따로 둠.
	// 카메라도 이 좌표를 따라가서 같이 부드럽게 움직임.
	float GetVisualX() const { return visualX; }
	float GetVisualY() const { return visualY; }
	int GetBobOffset() const;

private:
	// 방향키 입력을 받아 이동을 시도하는 함수.
	void Move(float deltaTime);
	void ReadMoveClick();
	std::vector<Craft::Vector2> clickPath;
	size_t clickPathIndex = 0;

	// 바라보는 방향으로 총을 쏨.
	// 근접 공격은 조용하지만 약하고, 총은 강한 대신 소리가 크게 나서
	// 주변 좀비를 전부 불러들임. 그 맞바꿈이 이 무기의 핵심.
	void FireGun();

	// 매 프레임 실제 좌표 쪽으로 조금씩 당겨옴.
	void UpdateVisualPosition(float deltaTime);

private:
	float visualX = 0.0f;
	float visualY = 0.0f;
	bool visualReady = false;
	// 클수록 빨리 따라붙음. 이동 간격 0.12초에 맞춰 짧게 잡음.
	float visualFollowSpeed = 38.0f;
	float bobTimer = 0.0f;

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

	// 총알이 날아가는 최대 칸 수. 벽이나 좀비를 만나면 거기서 멈춤.
	int gunRange = 9;
	int gunDamage = 50;
	// 근접 공격이 내는 소리보다 훨씬 넓게 퍼짐.
	int gunNoiseRange = 14;
	float gunInterval = 0.6f;

	// 총구에서 탄착점까지 잠깐 선을 그리기 위한 값들.
	Craft::Vector2 gunShotEnd = Craft::Vector2::Zero;
	float gunEffectTimer = 0.0f;
};
