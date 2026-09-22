#include "Player.h"
#include "../Audio/AudioSystem.h"
#include <Engine/Engine.h>
#include <Input/Input.h>
#include <Level/FloorLevel.h>
#include <Renderer/Renderer.h>
#include <Windows.h>
#include <PathFinding/AStar.h>
#include <cmath>

using namespace Craft;

Player::Player(const Vector2& position)
	// 부모 생성자에 값을 넘김.
	: Actor("P", position, Color::ClothLight)
{
	// 바닥보다 위에 그려지도록.
	sortingOrder = 5;
}

void Player::Tick(float deltaTime)
{
	Actor::Tick(deltaTime);
	if (!visualReady) UpdateVisualPosition(0.0f);
	ReadMoveClick();
	Move(deltaTime);
	UpdateVisualPosition(deltaTime);

	// 실제 위치와 그리기 위치가 다를 때만 통통 튀는 모션을 재생함.
	const float visualDistance =
		std::abs(static_cast<float>(position.x) - visualX)
		+ std::abs(static_cast<float>(position.y) - visualY);

	if (visualDistance > 0.04f)
	{
		bobTimer += deltaTime * 18.0f;
	}
	else
	{
		bobTimer = 0.0f;
	}
	
	// attackTime은 다음 공격까지 남은 시간.
	// 공격하면 0.4초로 설정하고, 매 프레임 줄어듦.
	// 빗나가도 쿨타임이 적용되며, 쿨타임 동안 누른 Z는 무시 됨.
	if (attackTimer > 0.0f)
	{
		attackTimer -= deltaTime;
	}

	if (attackEffectTimer > 0.0f)
	{
		attackEffectTimer -= deltaTime;
	}

	// Z키 한 번 누를 때마다 바라보는 방향의 바로 앞 칸에 피해 25를 줌.
	// 꾹 누르고 잇어도 연속 공격하진 않음.
	if (Input::Get().GetKeyDown('Z') && attackTimer <= 0.0f)
	{
		std::shared_ptr<FloorLevel> floor =
			Cast<FloorLevel>(GetOwner());

		if (floor)
		{
			Vector2 attackPosition = position + facingDirection;
			attackHit = floor->AttackZombieAt(attackPosition, 25);
			attackEffectPosition = attackPosition;
			attackEffectTimer = 0.12f;
		}
		
		attackTimer = attackInterval;
	}

	if (Input::Get().GetKeyDown('X') && attackTimer <= 0.0f)
	{
		FireGun();
	}

	if (gunEffectTimer > 0.0f)
	{
		gunEffectTimer -= deltaTime;
	}

	if (Input::Get().GetKeyDown('F'))
	{
		std::shared_ptr<FloorLevel> floor =
			Cast<FloorLevel>(GetOwner());

		if (floor)
		{
			// 키보드 이동은 마지막 이동 방향이 문을 향하지만,
			// 마우스 경로 이동은 문 앞에 도착할 때 다른 방향을
			// 바라보고 끝날 수 있음. 먼저 정면을 시도하고,
			// 정면에 문이 없으면 인접한 네 칸에서 문을 찾아 상호작용함.
			const Vector2 frontPosition = position + facingDirection;
			const TileType frontTile =
				floor->GetTile(frontPosition.x, frontPosition.y);

			if (frontTile == TileType::ClosedDoor
				|| frontTile == TileType::OpenDoor)
			{
				floor->ToggleDoorAt(frontPosition);
				return;
			}

			const Vector2 directions[] =
			{
				Vector2(0, -1),
				Vector2(0, 1),
				Vector2(-1, 0),
				Vector2(1, 0)
			};

			for (const Vector2& direction : directions)
			{
				const Vector2 doorPosition = position + direction;
				const TileType tile =
					floor->GetTile(doorPosition.x, doorPosition.y);

				if (tile != TileType::ClosedDoor
					&& tile != TileType::OpenDoor)
				{
					continue;
				}

				facingDirection = direction;
				floor->ToggleDoorAt(doorPosition);
				break;
			}
		}
	}
}

// 명중 여부에 따라 문자와 색을 고름.
void Player::Draw()
{
	std::shared_ptr<FloorLevel> floor =
		Cast<FloorLevel>(GetOwner());

	if (!floor)
		return;

	// The sheet rows are front, back, left, right. In the map, +Y is the
	// direction toward the camera, so +Y must use the front-facing frame.
	const int spriteDirection =
		facingDirection.y > 0 ? 0
		: facingDirection.y < 0 ? 1
		// The source's third row is the character's left-side profile,
		// which faces +X on screen; the fourth row faces -X.
		: facingDirection.x > 0 ? 2 : 3;
	const int spriteFrame = bobTimer > 0.0f
		? static_cast<int>(std::fmod(bobTimer * 0.9f, 3.0f))
		: 0;

	floor->DrawCharacter(
		visualX, visualY, CharacterSprite::Survivor, sortingOrder,
		GetBobOffset(), spriteDirection, spriteFrame);

	// 들고 있는 총이자 방향 표시.
	// 탄약이 남았으면 밝게, 다 떨어졌으면 어둡게 보여서 상태도 같이 읽힘.
	floor->DrawFacingMarker(
		visualX, visualY,
		facingDirection.x, facingDirection.y,
		floor->GetRunState().ammoCount > 0
			? Craft::Color::WallTop : Craft::Color::ClothDark,
		sortingOrder + 1);

	// 총을 쏜 직후 총구에서 탄착점까지 잠깐 선이 남음.
	if (gunEffectTimer > 0.0f)
	{
		Vector2 cursor = position;

		for (int step = 0; step < 16 && cursor != gunShotEnd; ++step)
		{
			cursor = cursor + facingDirection;

			Renderer::Get().Submit(
				facingDirection.y == 0 ? "-" : "|",
				floor->WorldToScreen(cursor),
				Color::WallTop, sortingOrder + 1);
		}
	}

	if (attackEffectTimer > 0.0f)
	{
		Renderer::Get().Submit(
			attackHit ? "*" : "-",
			floor->WorldToScreen(attackEffectPosition),
			attackHit ? Color::WallTop : Color::ClothDark,
			6);
	}
}

int Player::GetBobOffset() const
{
	if (bobTimer <= 0.0f)
	{
		return 0;
	}

	// 바닥에서 살짝 떠올랐다가 다시 내려오는 1~2픽셀 바운스.
	return -static_cast<int>(
		std::round(std::abs(std::sin(bobTimer)) * 2.0f));
}

void Player::ReadMoveClick()
{
	COORD mouse = {};
	if (!Input::Get().GetMouseClick(mouse)) return;
	clickPath.clear();
	clickPathIndex = 0;
	Vector2 pixel;
	Vector2 target;
	auto floor = Cast<FloorLevel>(GetOwner());
	if (!floor || !Renderer::Get().ConsoleToPixel(mouse, pixel)
		|| !floor->PickMoveTarget(pixel, target)) return;
	AStar pathfinder;
	clickPath = pathfinder.FindPath(position, target, *floor, false);
	clickPathIndex = clickPath.size() > 1 ? 1 : clickPath.size();
	if (clickPathIndex < clickPath.size())
	{
		// 대기 중 쌓인 시간이 첫 클릭 이동에 적용되지 않게 한다.
		moveTimer = 0.0f;
	}
}

void Player::Move(float deltaTime)
{
	// 이동 간격이 찰 때까지 시간을 모음.
	moveTimer += deltaTime;

	// 눌린 방향키에서 이동 방향 구하기.
	Vector2 direction = Vector2::Zero;

	// 한 번에 한 방향으로만 간다.
	if (Input::Get().GetKey(VK_LEFT))
	{
		direction = Vector2(-1, 0);
	}
	else if (Input::Get().GetKey(VK_RIGHT))
	{
		direction = Vector2(1, 0);
	}
	else if (Input::Get().GetKey(VK_UP))
	{
		direction = Vector2(0, -1);
	}
	else if (Input::Get().GetKey(VK_DOWN))
	{
		direction = Vector2(0, 1);
	}

	if (direction != Vector2::Zero)
	{
		clickPath.clear();
		clickPathIndex = 0;
	}
	else if (clickPathIndex < clickPath.size())
	{
		direction = clickPath[clickPathIndex] - position;
	}
	if (moveTimer < moveInterval) return;

	// 눌린 키가 없으면 종료.
	// 키를 눌렀는데 바로 안 움직이는 문제 방지.
	// 이동 할 때 마다 moveTimer을 0.0f로 되돌림.
	if (direction == Vector2::Zero)
	{
		return;
	}

	// 방향키를 누르면 그 방향을 기억함.
	facingDirection = direction;

	// 이동을 시도했으므로 타이머 초기화.
	// 벽을 향해 계속 누르고 있어도 시간이 안 쌓임.
	// 그러다 방향을 틀면 정상속도로 움직임.
	// 이 초기화를 뒤로 미루면, 방향 트는 순간 여러 칸을 튕겨나가게 됨.
	moveTimer = 0.0f;

	// 이 층에게 이동 가능한지 물어봄.
	// RTTI가 처음 작동하는 자리.
	// GetOwner()은 shared_ptr
	std::shared_ptr<FloorLevel> floor = Cast<FloorLevel>(GetOwner());

	if (!floor)
	{
		return;
	}

	Vector2 nextPosition = position + direction;

	if (!floor->IsWalkable(nextPosition)
		|| floor->HasZombieAt(nextPosition))
	{
		clickPath.clear();
		clickPathIndex = 0;
		return;
	}

	SetPosition(nextPosition);

	// 발소리. 재생 간격은 AudioSystem이 관리해서 뭉개지지 않음.
	AudioSystem::Get().Play(AudioSystem::SoundId::Move);

	if (clickPathIndex < clickPath.size()) ++clickPathIndex;
	floor->PickUpMedicineAt(nextPosition);

	// 이동한 칸이 Exit면 새 FloorLevel을 다음 레벨로 예약.
	if (floor->GetTile(nextPosition.x, nextPosition.y) == TileType::Exit)
	{
		floor->MoveToNextFloor();
	}

}

void Player::UpdateVisualPosition(float deltaTime)
{
	// 처음 한 번은 실제 좌표에 그대로 맞춤.
	// 안 그러면 화면 왼쪽 위에서 미끄러져 들어오는 것처럼 보임.
	if (!visualReady)
	{
		visualX = static_cast<float>(position.x);
		visualY = static_cast<float>(position.y);
		visualReady = true;
		return;
	}

	// 남은 거리의 일정 비율씩 좁힘.
	// 처음엔 빠르고 도착할수록 느려져서 딱 붙는 느낌이 남.
	float factor = 1.0f - std::exp(-visualFollowSpeed * deltaTime);

	if (factor > 1.0f)
	{
		factor = 1.0f;
	}

	visualX += (static_cast<float>(position.x) - visualX) * factor;
	visualY += (static_cast<float>(position.y) - visualY) * factor;
}

void Player::FireGun()
{
	std::shared_ptr<FloorLevel> floor = Cast<FloorLevel>(GetOwner());

	if (!floor)
	{
		return;
	}

	RunState& runState = floor->GetRunState();

	// 탄약이 없으면 방아쇠만 당기고 아무 일도 일어나지 않음.
	if (runState.ammoCount <= 0)
	{
		return;
	}

	--runState.ammoCount;
	attackTimer = gunInterval;

	// 바라보는 방향으로 한 칸씩 나아가며 처음 만나는 것에 맞음.
	Vector2 shot = position;

	for (int step = 0; step < gunRange; ++step)
	{
		Vector2 next = shot + facingDirection;

		// 벽이나 닫힌 문에 막히면 거기서 멈춤.
		if (!floor->IsWalkable(next))
		{
			break;
		}

		shot = next;

		// 좀비를 맞히면 관통하지 않고 멈춤.
		if (floor->AttackZombieAt(shot, gunDamage))
		{
			break;
		}
	}

	gunShotEnd = shot;
	gunEffectTimer = 0.15f;

	AudioSystem::Get().Play(AudioSystem::SoundId::Shot);

	// 총성은 근접 공격과 달리 멀리까지 퍼져서 좀비를 불러들임.
	floor->MakeNoise(position, gunNoiseRange);
}
