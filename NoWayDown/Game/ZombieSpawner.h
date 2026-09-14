#pragma once

class FloorLevel;
struct RunState;

// 좀비를 "언제, 어디에" 만들지만 담당함.
// 좀비의 행동은 Zombie가, 지형과 좌표 질문은 FloorLevel이 맡음.
//
// FloorLevel에 흩어져 있던 세 가지 일을 한곳에 모은 것.
// 세 시점이 모두 다르기 때문에 Actor가 아니라 FloorLevel이 소유하는
// 평범한 객체로 두고, 필요한 시점에 직접 부름.
class ZombieSpawner
{
public:
	// 층이 시작될 때. 맵 파일에 적힌 좌표마다 좀비를 만듦.
	void SpawnMapZombies(FloorLevel& floor);

	// 매 프레임. 이전 층에서 따라온 좀비의 등장 시간을 세고
	// 때가 된 좀비를 한 마리 만듦.
	void Update(float deltaTime, FloorLevel& floor, RunState& runState);

	// 층을 떠나기 직전. 따라올 좀비를 RunState에 기록함.
	void SaveFollowingZombies(FloorLevel& floor, RunState& runState);

private:
	// 좀비가 처음 추격을 시작한 층에서 몇 층까지 따라오는지.
	// 이 값을 넘으면 그 좀비는 더 이상 따라오지 않음.
	int maxChaseFloors = 3;
};
