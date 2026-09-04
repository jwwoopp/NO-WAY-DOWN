#include "Player.h"
#include <Input/Input.h>
#include <Level/FloorLevel.h>
#include <Windows.h>

using namespace Craft;

Player::Player(const Vector2& position)
	// 부모 생성자에 값을 넘김.
	: Actor("P", position, Color::Green)
{
	// 바닥보다 위에 그려지도록.
	sortingOrder = 5;
}

void Player::Tick(float deltaTime)
{
	Actor::Tick(deltaTime);

	ProcessMove(deltaTime);
	// 나중에: ProcessExit();
	// 나중에: ProcessQuit();
}

void Player::ProcessMove(float deltaTime)
{
	// 이동 간격이 찰 때까지 시간을 모음.
	moveTimer += deltaTime;

	if (moveTimer < moveInterval)
	{
		return;
	}

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

	// 눌린 키가 없으면 종료.
	// 키를 눌렀는데 바로 안 움직이는 문제 방지.
	// 이동 할 때 마다 moveTimer을 0.0f로 되돌림.
	if (direction == Vector2::Zero)
	{
		return;
	}

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

	if (!floor->IsWalkable(nextPosition))
	{
		return;
	}

	SetPosition(nextPosition);

}