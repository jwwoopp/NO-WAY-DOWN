#pragma once

struct RunState
{
	// currentFloor - 현재 층.
	int currentFloor = 1;
	// health - 층이 바뀌어도 유지될 체력.
	int health = 100;

	bool debugMode = false;
	bool isGameOver = false;
	bool isCleared = false;
};