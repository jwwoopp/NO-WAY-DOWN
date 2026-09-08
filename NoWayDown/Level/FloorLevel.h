#pragma once

#include <Level/Level.h>
#include <Math/Vector2.h>
#include <Map/TileType.h>
#include <Game/RunState.h>
#include <memory>
#include <string>
#include <vector>

// NO WAY DOWN의 한 층을 담당하는 레벨.
class Player;
class FloorLevel : public Craft::Level
{
	TYPE_DECLARATIONS(FloorLevel, Level)

public:
	// FloorLevel을 만들 때 공유할 RunState를 반드시 전달받겠다는 생성자 선언.
	// const와 &만 사용하므로, RunState 자체를 복사하지않고 shared_ptr만 안전하게 받음.
	FloorLevel(const std::shared_ptr<RunState>& newRunState);

	// 해당 좌표로 이동할 수 있는지 판정.
	// Player가 이동 전에 물어봐야 하니까 public.
	bool IsWalkable(const Craft::Vector2& position) const;
	// 지형 검사와 좀비 검사를 분리했으므로 A*의 기존 지형 판단에는 영향이 없음.
	bool HasZombieAt(const Craft::Vector2& position) const;

	bool HasLineOfSight(
		const Craft::Vector2& from,
		const Craft::Vector2& to) const;

	// 층 전환은 FloorLevel이 담당하게 함.
	void MoveToNextFloor();

	// 해당 좌표의 타일 종류를 반환.
	TileType GetTile(int x, int y) const;

	int GetWidth() const { return width; }
	int GetHeight() const { return height; }

	Craft::Vector2 GetPlayerPosition() const;
	void DamagePlayer(int amount);
	bool IsDebugMode() const { return runState->debugMode; }

	bool AttackZombieAt(
		const Craft::Vector2& targetPosition,
		int damage);

private:
	// OnInitialized와 Draw는 Engine이 부르기 때문에 private.
	virtual void OnInitialized() override;
	virtual void Tick(float deltaTime) override;
	virtual void Draw() override;

	// 맵 파일을 읽어 tiles를 채우는 함수.
	void LoadMap(const std::string& filename);

private:
	// 생성자로 전달받은 shared_ptr을 이 변수에 저장.
	// FloorLevel이 바뀌어도 다음 FloorLevel에 같은 runState를 전달하면 진행 정보 유지.
	std::shared_ptr<RunState> runState;
	// 층의 정적 지형. 1차원 배열을 (y * width) + x로 접근.
	std::vector<TileType> tiles;
	// Level의 ActorList가 Player를 실제로 소유하고,
	// weak_ptr인 player은 그 Player의 위치를 찾기 위한 약한 역할만 보관.
	std::weak_ptr<Player> player;

	int width = 0;
	int height = 0;

	// 맵 파일에서 찾은 플레이어 시작 좌표.
	Craft::Vector2 playerStart;

	// 맵에는 좀비가 여럿 있을 수 있으므로 시작 좌표를 vector에 모음.
	std::vector<Craft::Vector2> zombieStarts;
};