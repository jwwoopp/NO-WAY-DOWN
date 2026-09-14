#pragma once

#include <Level/Level.h>
#include <Math/Vector2.h>
#include <Map/TileType.h>
#include <Game/RunState.h>
#include <Game/ZombieSpawner.h>
#include <Map/TileMap.h>
#include <Map/DoorSystem.h>
#include "../Renderer/IsometricRenderer.h"

#include <memory>
#include <string>
#include <vector>
#include <queue>

// NO WAY DOWN의 한 층을 담당하는 레벨.
class Player;
class Zombie;
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

	// int GetWidth() const { return width; }
	// int GetHeight() const { return height; }

	// 화면을 그릴 때 timeMap의 가로 세로 크기를 사용.
	int GetWidth() const { return tileMap.GetWidth(); }
	int GetHeight() const { return tileMap.GetHeight(); }

	Craft::Vector2 GetPlayerPosition() const;
	bool PickMoveTarget(const Craft::Vector2& pixel, Craft::Vector2& tile) const
	{
		// HUD occupies the first 18 terminal rows.
		if (pixel.y < 36) return false;
		tile = isometricRenderer.ScreenToWorld(pixel);
		if (tile.x < 0 || tile.y < 0 || tile.x >= GetWidth() || tile.y >= GetHeight()) return false;
		const int index = tile.y * GetWidth() + tile.x;
		return index < static_cast<int>(tileSeen.size()) && tileSeen[index] && IsWalkable(tile);
	}
	Craft::Vector2 WorldToScreen(const Craft::Vector2& position) const
	{
		return isometricRenderer.WorldToScreen(
			static_cast<float>(position.x) + 0.5f,
			static_cast<float>(position.y) + 0.5f
		);
	}

	void DamagePlayer(int amount);
	// 그리기용 좌표는 실수.
	// 칸 단위로 순간이동하면 딱딱해 보여서 액터가 부드럽게 따라온 위치를 넘김.
	void DrawCharacter(float worldX, float worldY,
		CharacterSprite sprite, int sortingOrder,
		int verticalOffset = 0,
		int direction = 0,
		int frame = 0) const
	{
		isometricRenderer.DrawCharacter(
			worldX, worldY, sprite, sortingOrder, verticalOffset,
			direction, frame);
	}
	// 바라보는 방향에 총열 겸 방향 표시를 그림.
	void DrawFacingMarker(float worldX, float worldY,
		int directionX, int directionY,
		Craft::Color color, int sortingOrder) const
	{
		isometricRenderer.DrawFacingMarker(
			worldX, worldY, directionX, directionY, color, sortingOrder);
	}

	bool UseMedicine();
	void PickUpMedicineAt(const Craft::Vector2& position);
	void ToggleDoorAt(const Craft::Vector2& position);
	// 좀비 문 타격 요청.
	bool HitDoorAt(const Craft::Vector2& position);
	// 문을 열 때 거리 6칸 이내의 좀비에게 소리를 전달.
	void MakeNoise(const Craft::Vector2& noisePosition, int range);

	// 그 칸에 핏자국을 남김. 한 번 생기면 그 층이 끝날 때까지 지워지지 않아
	// 어디서 싸웠는지가 지도처럼 남음.
	void AddBlood(const Craft::Vector2& position, int amount);
	bool IsDebugMode() const { return runState->debugMode; }
	bool IsTileSeen(int x, int y) const;

	bool AttackZombieAt(
		const Craft::Vector2& targetPosition,
		int damage);

	// 아래 넷은 ZombieSpawner가 물어보는 것들.
	// 좀비를 어디에 놓을지 정하려면 맵과 액터 상태를 알아야 함.
	const std::vector<Craft::Vector2>& GetZombieStarts() const
	{
		return zombieStarts;
	}

	Craft::Vector2 GetPlayerStart() const { return playerStart; }

	// 탄약처럼 층을 넘어도 유지되는 값은 RunState에 있음.
	RunState& GetRunState() const { return *runState; }

	// 살아 있는 좀비 목록.
	// actorList는 Level의 protected라 밖에서 못 보므로 걸러서 넘겨줌.
	std::vector<std::shared_ptr<Zombie>> GetActiveZombies() const;

	// start는 탐색을 시작할 입구 위치.
	// 빈칸을 찾으면 outPosition에 좌표를 넣고 true, 찾지 못하면 false 반환.
	bool FindSpawnPosition(
		const Craft::Vector2& start,
		Craft::Vector2& outPosition) const;

private:
	// OnInitialized와 Draw는 Engine이 부르기 때문에 private.
	virtual void OnInitialized() override;
	virtual void Tick(float deltaTime) override;
	virtual void Draw() override;
	void DrawMapOverlay();
	void UpdateCameraTwist(float deltaTime);

	// 맵 파일을 읽어 tiles를 채우는 함수.
	bool LoadMap(const std::string& filename);

private:
	// 생성자로 전달받은 shared_ptr을 이 변수에 저장.
	// FloorLevel이 바뀌어도 다음 FloorLevel에 같은 runState를 전달하면 진행 정보 유지.
	std::shared_ptr<RunState> runState;
	// 지형 담당 객체를 둠. 상속이 아니라 가지고 있는 관계.
	TileMap tileMap;
	DoorSystem doorSystem;
	IsometricRenderer isometricRenderer;

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
	// 바닥에 남아 있는 회복약들의 위치. 주운 약의 개수는 RunState에서 관리.
	std::vector<Craft::Vector2> medicinePositions;
	// 마지막 소리 위치를 !. 범위를 숫자로 표현.
	Craft::Vector2 lastNoisePosition = Craft::Vector2::Zero;
	int lastNoiseRange = 0;

	// 죽거나 탈출한 순간을 잠깐 보여준 뒤 결과 화면으로 넘어가려고 세는 시간.
	// 바로 넘기면 플레이어가 무슨 일이 일어났는지 못 봄.
	float endingDelayTimer = 0.0f;
	float endingDelay = 1.2f;

	// 이전 층 화면을 점점 어둡게 만들면서 흩어지듯 지워 잔상으로 보여줌.
	void DrawFloorTransition();

	// 시야 기억.
	// 지금 보이는 칸은 밝게, 지나온 칸은 어둡게 남기고, 안 가본 칸은 안 그림.
	// 플레이어가 칸을 옮겼을 때만 시야를 다시 긋고, 밝기는 매 프레임 부드럽게 변함.
	void UpdateVisibility(float deltaTime);

	// HasLineOfSight와 달리 목표 칸이 벽이어도 보이는 것으로 침.
	// 벽이 안 보이면 방 모양 자체가 안 읽힘.
	bool IsTileVisibleFrom(
		const Craft::Vector2& from, int toX, int toY) const;

public:
	// 그 칸이 지금 화면에 밝게 보이는지. 좀비를 그릴지 판단하는 데 씀.
	bool IsTileLit(int x, int y) const;

private:
	// 칸별 밝기. 0이면 안 그리고, rememberedLight면 지나온 기억, 1이면 지금 보임.
	// 칸마다 쌓인 핏자국 양. 0이면 없음.
	std::vector<unsigned char> bloodLevel;

	std::vector<float> tileLight;
	std::vector<bool> tileSeen;
	std::vector<bool> tileVisibleNow;
	Craft::Vector2 lastVisibilityOrigin = Craft::Vector2(-1, -1);
	bool showMap = false;

	// 맵을 네 공간으로 나눠 공간을 옮길 때 시점 방향을 살짝 바꿈.
	// 타일 좌표·충돌에는 영향을 주지 않고 그리기 각도만 바뀜.
	int cameraTwistZone = -1;
	float cameraTwistTarget = 0.14f;
	float cameraTwistTimer = 0.0f;

	// 화면에 가로 10칸이 보이므로 반경이 그보다 크면 시야 효과가 안 보임.
	int sightRadius = 6;
	float rememberedLight = 0.42f;
	float lightFadeSpeed = 6.0f;

	// 좀비를 언제 어디에 만들지는 이쪽이 전담함.
	// 층 시작, 매 프레임, 층 이탈 세 시점에 FloorLevel이 직접 부름.
	ZombieSpawner zombieSpawner;
};
