#pragma once

#include <Math/Vector2.h>
#include <Math/Color.h>

// 타이틀과 엔딩 화면에 쓰는 측면 뷰 배경 조각들.
// 게임 플레이 화면은 탑뷰지만 이 두 화면은 연출용이라 측면으로 그림.
// 어두운 배경에 밝은 바닥선 하나를 긋고 그 위에 문과 사람을 세우는 구도.
// 사람은 게임 안 스프라이트와 같은 크림색 몸에 회록색 배낭을 써서
// 같은 인물로 보이게 맞춤.
namespace SceneArt
{
	// 화면 축에 나란한 사각형을 채움. 조각들이 공통으로 쓰는 기본 도구.
	void FillRect(
		int x, int y, int width, int height,
		Craft::Color color, int sortingOrder = 5);

	// 화면 전체를 가로지르는 바닥선.
	void DrawFloorLine(int y, Craft::Color color, int sortingOrder = 5);

	// 나무 문. bottomY가 바닥에 닿는 줄, x는 왼쪽 끝.
	// open이면 문짝이 열리고 가운데가 어둠으로 뚫림.
	void DrawDoor(int x, int bottomY, bool open, int sortingOrder = 5);

	// 오른쪽을 보고 선 사람의 측면 모습.
	void DrawStandingPerson(int x, int bottomY, int sortingOrder = 6);

	// 쓰러진 사람. 사망 엔딩에 씀.
	void DrawFallenPerson(int x, int bottomY, int sortingOrder = 6);

	// 빈 공간을 채우는 소품들. 어둡게 그려서 사람과 경쟁하지 않음.
	void DrawPanelProp(int x, int bottomY, int sortingOrder = 4);
	void DrawValveProp(int x, int bottomY, int sortingOrder = 4);

	// 배치를 계산할 때 쓰는 크기들.
	int DoorWidth();
	int DoorHeight();
	int PersonWidth();
	int PersonHeight();
	int FallenWidth();
	int FallenHeight();
}
