#pragma once
#include <Game/ZombieData.h>
#include <vector>
#include <Windows.h>

struct RunState
{
	// 층을 넘어가기 직전 화면을 통째로 복사해 둔 것.
	// 새 층이 이걸 점점 어둡게 지우면서 잔상처럼 보이게 함.
	// Level이 교체돼도 살아남아야 해서 RunState가 들고 있음.
	std::vector<CHAR_INFO> transitionFrame;
	// 잔상이 남아 있는 시간. 0이 되면 잔상이 끝남.
	float transitionTimer = 0.0f;
	float transitionDuration = 0.7f;

	// currentFloor - 현재 층.
	int currentFloor = 1;
	// health - 층이 바뀌어도 유지될 체력.
	int health = 100;
	// medicine - 층이 바뀌어도 가져갈 수 있는 회복약.
	int medicineCount = 0;
	// 층이 바뀌어도 유지되는 탄약. 보급이 없어서 쓸 때마다 줄기만 함.
	int ammoCount = 6;
	std::vector<ZombieData> incomingZombies;

	bool debugMode = false;
	bool isGameOver = false;
	bool isCleared = false;
};