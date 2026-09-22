#include "Zombie.h"
#include "../Audio/AudioSystem.h"
#include <Level/FloorLevel.h>
#include <Renderer/Renderer.h>
#include <cstdlib>
#include <cmath>

using namespace Craft;

Zombie::Zombie(const Vector2& position)
	: Actor("Z", position, Color::Danger)
{
	// 바닥보다 위에 그리되 Player의 5보다 낮게 둠.
	sortingOrder = 4;
}

// 기존 생성자에게 기본 외형과 위치 설정을 맡기는 위임 생성자.
// 저장된 체력과 최초 추격 층을 복원함.
// 따라서 이전 층에서 다친 좀비가 체력 100으로 되살아나는 걸 막음.
Zombie::Zombie(const Vector2& position, const ZombieData& data)
	: Zombie(position)
{
	health = data.health;
	originFloor = data.originFloor;
}

// 매 프레임 부모 Actor의 기본 처리 후 추격 시간 계산함.
void Zombie::Tick(float deltaTime)
{
	Actor::Tick(deltaTime);

	if (doorAttackTimer > 0.0f)
	{
		doorAttackTimer -= deltaTime;
	}

	if (doorRestTimer > 0.0f)
	{
		doorRestTimer -= deltaTime;
	}

	if (interestTimer > 0.0f)
	{
		interestTimer -= deltaTime;
	}

	DetectPlayer();
	Chase(deltaTime);
	UpdateVisualPosition(deltaTime);

	const float visualDistance =
		std::abs(static_cast<float>(position.x) - visualX)
		+ std::abs(static_cast<float>(position.y) - visualY);

	if (visualDistance > 0.04f)
	{
		bobTimer += deltaTime * 12.0f;
	}
	else
	{
		bobTimer = 0.0f;
	}
}

void Zombie::Draw()
{
	std::shared_ptr<FloorLevel> floor =
		Cast<FloorLevel>(GetOwner());

	if (!floor)
		return;

	const int spriteDirection =
		facingDirection.y > 0 ? 0
		: facingDirection.y < 0 ? 1
		: facingDirection.x > 0 ? 2 : 3;
	const int spriteFrame = bobTimer > 0.0f
		? static_cast<int>(std::fmod(bobTimer * 0.6f, 3.0f))
		: 0;

	// F1 키를 누르면 마지막으로 계산한 A* 경로가 표시 됨.
	if (floor && floor->IsDebugMode())
	{
		for (const Vector2& pathPosition : latestPath)
		{
			Renderer::Get().Submit(
				"+", floor->WorldToScreen(pathPosition), Color::DoorEdge, 3);
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

	// 시야 밖의 좀비는 그리지 않음. 어디 있는지 모르는 편이 훨씬 무서움.
	if (floor->IsTileLit(position.x, position.y))
	{
		floor->DrawCharacter(
			visualX, visualY, CharacterSprite::Zombie, sortingOrder,
			GetBobOffset(), spriteDirection, spriteFrame);
	}
	if (floor->IsDebugMode())
	{
		Vector2 label = floor->WorldToScreen(position);
		label.y -= 22;
		Renderer::Get().Submit(stateText, label, Color::Skin, 6);
	}

}

int Zombie::GetBobOffset() const
{
	if (bobTimer <= 0.0f)
	{
		return 0;
	}

	return -static_cast<int>(
		std::round(std::abs(std::sin(bobTimer)) * 2.0f));
}

void Zombie::Chase(float deltaTime)
{
	moveTimer += deltaTime;

	if (moveTimer < setting.moveInterval)
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

	if (!canSeePlayer && state == ZombieState::Chase)
	{
		state = ZombieState::Search;
	}

	if (state == ZombieState::Idle)
	{
		latestPath.clear();
		return;
	}

	// 목표 위치에 도착했거나, 새 단서 없이 관심 시간이 끝나면 대기 상태로 돌아감.
	// 좀비 연속 타격과 휴식 상태만 초기화하고, 문에 누적된 피해는 그대로 남김.
	// 플레이어가 계속 보이면 위쪽 코드에서 관심 시간을 8초로 갱신하므로 추격 포기하지 않음.
	
	if (state == ZombieState::Search
		&& (position == targetPosition || interestTimer <= 0.0f))
	{
		state = ZombieState::Idle;
		latestPath.clear();

		consecutiveDoorHits = 0;
		doorAttackTimer = 0.0f;
		doorRestTimer = 0.0f;
		return;
	}

	latestPath = pathFinder.FindPath(
		position,
		targetPosition,
		*floor,
		true);

	// 경로가 두 칸보다 짧으면 이미 도착햇거나 길을 찾지 못한 경우라 이동하지 못함.
	if (latestPath.size() < 2)
	{
		return;
	}

	Vector2 nextPosition = latestPath[1];
	facingDirection = nextPosition - position;

	// 다음 칸이 닫힌 문이면 공격만 하고, 그 칸으로 이동하진 않음.
	if (floor->GetTile(nextPosition.x, nextPosition.y)
		== TileType::ClosedDoor)
	{
		AttackDoor(nextPosition);
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

	// 가까운 좀비의 발소리만 들림.
	// 맵 반대편 좀비까지 들리면 어디에 있는지 짐작할 수 없어 소음만 됨.
	{
		const Vector2 playerPosition = floor->GetPlayerPosition();
		const int distance =
			std::abs(position.x - playerPosition.x)
			+ std::abs(position.y - playerPosition.y);

		if (distance <= 10)
		{
			AudioSystem::Get().Play(AudioSystem::SoundId::ZombieStep);
		}
	}

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

void Zombie::HearNoise(const Vector2& noisePosition)
{
	// 추격 중에는 기존 추격을 우선함.
	if (!IsActive() || state == ZombieState::Chase)
	{
		return;
	}

	targetPosition = noisePosition;
	// 나머지는 소리가 난 위치를 수색 목표르 삼음.
	state = ZombieState::Search;
	interestTimer = setting.interestDuration;
	latestPath.clear();
}

void Zombie::AttackDoor(const Vector2& doorPosition)
{
	// 한 번 때리면 다음 타격까지 1초를 기다림.
	if (interestTimer <= 0.0f
		|| doorAttackTimer > 0.0f || doorRestTimer > 0.0f)
	{
		return;
	}

	std::shared_ptr<FloorLevel> floor = Cast<FloorLevel>(GetOwner());

	if (!floor || floor->GetTile(doorPosition.x, doorPosition.y)
		!= TileType::ClosedDoor)
	{
		return;
	}

	bool destroyed = floor->HitDoorAt(doorPosition);

	// 부서지는 순간과 그냥 두드리는 소리를 구분해야
	// 문 너머에서 무슨 일이 벌어지는지 소리만으로 알 수 있음.
	AudioSystem::Get().Play(destroyed
		? AudioSystem::SoundId::DoorBreak
		: AudioSystem::SoundId::DoorKnock);
	doorAttackTimer = setting.doorAttackInterval;
	++consecutiveDoorHits;

	if (destroyed)
	{
		consecutiveDoorHits = 0;
		latestPath.clear();
	}
	// 연속 3회 이후에는 2초 동안 쉼.
	else if (consecutiveDoorHits >= setting.doorHitsBeforeRest)
	{
		consecutiveDoorHits = 0;
		doorRestTimer = setting.doorRestDuration;
	}
}

void Zombie::DetectPlayer()
{
	canSeePlayer = false;

	std::shared_ptr<FloorLevel> floor =
		Cast<FloorLevel>(GetOwner());

	if (!floor)
		return;

	Vector2 playerPosition = floor->GetPlayerPosition();

	int distance =
		std::abs(position.x - playerPosition.x)
		+ std::abs(position.y - playerPosition.y);

	canSeePlayer = distance <= setting.detectionRange
		&& floor->HasLineOfSight(position, playerPosition);

	if (canSeePlayer)
	{
		// 추격을 새로 시작하는 순간에만 울림. 매 프레임 울리면 소음이 됨.
		if (state != ZombieState::Chase)
		{
			AudioSystem::Get().Play(AudioSystem::SoundId::Danger);
		}

		state = ZombieState::Chase;
		targetPosition = playerPosition;
		interestTimer = setting.interestDuration;
	}
}

void Zombie::UpdateVisualPosition(float deltaTime)
{
	if (!visualReady)
	{
		visualX = static_cast<float>(position.x);
		visualY = static_cast<float>(position.y);
		visualReady = true;
		return;
	}

	// 좀비는 0.5초마다 한 칸씩 움직이므로 천천히 따라오게 해야
	// 순간이동이 아니라 걸어오는 것처럼 보임.
	float factor = visualFollowSpeed * deltaTime;

	if (factor > 1.0f)
	{
		factor = 1.0f;
	}

	visualX += (static_cast<float>(position.x) - visualX) * factor;
	visualY += (static_cast<float>(position.y) - visualY) * factor;
}
