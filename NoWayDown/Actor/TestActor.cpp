#include "TestActor.h"
#include <Input/Input.h>
#include <Windows.h>

using namespace Craft;

TestActor::TestActor()
	: Actor("P", Vector2(5, 5), Color::Green)
{
	// 기본 배경보다 플레이어가 앞에 그려지게 됨.
	sortingOrder = 5;
}

void TestActor::Tick(float deltaTime)
{
	Actor::Tick(deltaTime);

	const int size = 256;
	char fpsString[size] = {};

	// 값을 글자로 만듦.
	sprintf_s(
		fpsString,
		size,
		"dt: %f | fps: %.1f",
		deltaTime,
		(1.0f / deltaTime)
	);

	// 콘솔 창 제목 표시줄에 글자를 씀.
	SetConsoleTitleA(fpsString);

	// GetKey가 아니라 GetKeyDown을 써야함.
	// 누른 순간 딱 한번만 반응.

	if (Input::Get().GetKeyDown(VK_ESCAPE))
	{
		QuitGame();
	}

	// 이동 구현.
	if (Input::Get().GetKey(VK_LEFT) && position.x > 0)
	{
		// x가 0보다 작아져 화면 밖으로 나가지 않게 제한.
		position.x -= 1;
	}

	if (Input::Get().GetKey(VK_RIGHT) && position.x < 39)
	{
		// 오른쪽 키를 누르면 x가 증가.
		// 임시 콘솔 너비가 현재는 40이라 하드코딩으로 좌표 39 제한.
		position.x += 1;
	}

	if (Input::Get().GetKey(VK_UP) && position.y > 0)
	{
		// UP 키를 누르면 y가 하나씩 감소.
		// position.y > 0 : y가 -1이 되어 화면 밖으로 나가는 것을 방지.
		position.y -= 1;
	}

	if (Input::Get().GetKey(VK_DOWN) && position.y < 24)
	{
		// DOWN 키를 누르면 y가 하나씩 증가.
		// 임시 콘솔 높이가 현재는 24라 하드코딩으로 좌표 25 제한.
		position.y += 1;
	}



}
