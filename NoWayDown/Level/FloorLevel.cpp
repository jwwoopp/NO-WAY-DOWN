#include "FloorLevel.h"
#include "../Audio/AudioSystem.h"
#include <Actor/Player.h>
#include <Actor/Zombie.h>

#include <Engine/Engine.h>
#include <Input/Input.h>
#include <Renderer/Renderer.h>
#include <Map/MapLoader.h>

#include "../Renderer/IsometricRenderer.h"
#include <PathFinding/AStar.h>
#include <UI/GameHUD.h>
#include "EndingLevel.h"

#include <Windows.h>
#include <iostream>
#include <cstdlib>
#include <utility>
#include <algorithm>

using namespace Craft;

namespace
{
	// 팔레트 색을 한 단계 어둡게 바꾸는 표.
	// 중요한 건 "기억에 남은 바닥"이 "지금 보이는 벽"과 같은 색이 되면 안 된다는 것.
	// 그러면 벽인지 지나온 길인지 구분이 안 됨.
	// 그래서 바닥(3)은 벽 윗면(2)을 건너뛰고 더 어두운 1로 내림.
	const int dimTable[16] =
	{
		0, 0, 14, 1, 2, 4, 5, 9, 7, 14, 9, 10, 1, 4, 0, 1
	};

	Color DimOnce(Color color)
	{
		return static_cast<Color>(
			dimTable[static_cast<int>(color) & 0x0F]);
	}

	// 지금 보이는 칸이면 원래 색, 기억에만 남은 칸이면 한 단계 어둡게.
	Color Shade(bool bright, Color base)
	{
		return bright ? base : DimOnce(base);
	}

	// 밝기가 거의 0이면 아예 그리지 않아 검게 남음.
	bool ShadeByLight(float light, Color base, Color& out)
	{
		if (light < 0.06f)
		{
			return false;
		}

		out = light >= 0.72f ? base : DimOnce(base);
		return true;
	}
}

FloorLevel::FloorLevel(const std::shared_ptr<RunState>& newRunState)
	// 전달받은 shared_ptr를 멤버 변수 runState에 저장하는 생성자 초기화 목록.
	// 두 shared_ptr이 같은 RunState를 가리키므로
	// Level이 바뀌어도 동일한 진행데이터를 넘길 수 있다.
	: runState(newRunState)
{
}

void FloorLevel::MoveToNextFloor()
{	
	if (floorTransitionRequested || runState->isGameOver || runState->isCleared)
	{
		return;
	}

	floorTransitionRequested = true;

	// 임시로 12층 출구에서 클리어. 옥상 장면은 이후 연결.
	if (runState->currentFloor >= 12)
	{
		runState->isCleared = true;
		return;
	}

	for (ZombieData& data : runState->incomingZombies)
	{
		if (data.destinationFloor == runState->currentFloor)
		{
			// 아직 도착하지 않은 좀비는 목적지를 다음 층으로 바꿈.
			data.destinationFloor = runState->currentFloor + 1;

			if (data.arrivalDelay < 0.0f)
			{
				data.arrivalDelay = 0.0f;
			}

			// 남은 시간에 3초를 더함.
			data.arrivalDelay += 3.0f;
		}
	}

	// 현재 층 번호로 좀비 정보를 기록한 다음, 층 번호를 올리는 순서.
	// 12층 탈출은 위쪽에서 return하므로 추격 기록을 새로 만들지 않음.
	zombieSpawner.SaveFollowingZombies(*this, *runState);
	++runState->currentFloor;

	// 층을 넘어가기 직전 화면을 복사해 둠.
	// Tick 중이라 Renderer에는 직전 프레임 화면이 그대로 남아 있음.
	// 새 층이 이걸 잔상으로 지우면서 시작함.
	AudioSystem::Get().Play(AudioSystem::SoundId::FloorTransition);

	Renderer::Get().CopyCurrentFrame(runState->transitionFrame);
	runState->transitionTimer = runState->transitionDuration;

	Engine::Get().AddNewLevel<FloorLevel>(runState);
}

void FloorLevel::DamagePlayer(int amount)
{

	// 탈출한 프레임에 좀비의 공격 처리가 남아있어도 피해를 받지 않게 함.
	if (floorTransitionRequested || runState->isGameOver || runState->isCleared)
	{
		return;
	}

	runState->health -= amount;

	if (runState->health <= 0)
	{
		runState->health = 0;
		runState->isGameOver = true;

		// 배경음을 끊고 사망음을 얹어야 순간이 확실하게 끊김.
		AudioSystem::Get().Stop(AudioSystem::SoundId::MainTheme);
		AudioSystem::Get().Play(AudioSystem::SoundId::GameOver);
	}
}
// FloorLevel이 현재 Actor를 하나씩 확인하고,
bool FloorLevel::AttackZombieAt(
	const Vector2& targetPosition,
	int damage)
{
	for (const std::shared_ptr<Actor>& actor : actorList)
	{
		std::shared_ptr<Zombie> zombie = Cast<Zombie>(actor);

		// 해당 좌표의 Zombie를 찾으면 피해를 줍니다.
		if (!zombie || !zombie->IsActive())
		{
			continue;
		}

		if (zombie->GetPosition() == targetPosition)
		{
			// 맞혔으면 true.
			zombie->TakeDamage(damage);

			// 맞은 자리에 핏자국을 남김. 죽으면 더 크게 번짐.
			// 지워지지 않아서 어디서 싸웠는지가 지도처럼 남음.
			AddBlood(targetPosition, zombie->GetHealth() <= 0 ? 3 : 1);

			return true;
		}
		// 빈 공간을 공격하면 false.
	}

	return false;
}

// 이 칸에 살아있는 좀비가 서있는 지 여부 확인하는 함수.
// Player가 이 검사를 해서 바닥이어도 좀비가 있으면 멈춤.
// A*는 이걸 사용하지 않기에 벽과 바닥을 판단하는 규칙은 그대로임.
// 따라서 Player의 이동만 막음.
bool FloorLevel::HasZombieAt(const Vector2& position) const
{
	for (const std::shared_ptr<Actor>& actor : actorList)
	{
		std::shared_ptr<Zombie> zombie = Cast<Zombie>(actor);

		if (zombie && zombie->IsActive()
			&& zombie->GetPosition() == position)
		{
			return true;
		}
	}

	return false;
}

void FloorLevel::OnInitialized()
{
	// 이걸 안찍으면 엔진이 매 프레임 또 부르고, 맵을 1초에 120번 다시 읽음.
	Level::OnInitialized();
	// 층수에 맞는 맵을 읽어 플레이어의 좌표를 찾음.
	std::string mapFilename = "Assets/BuildingMaps/Floor" + std::to_string(runState->currentFloor) + ".txt";
	
	if (!LoadMap(mapFilename))
	{
		std::cerr << "Failed to load map: " << mapFilename << "\n";
		Engine::Get().Quit();
		return;
	}

	// 그 좌표에 플레이어를 생성함.
	// SpawnActor<Player>(playerStart);
	player = SpawnActor<Player>(playerStart);
	
	// 배경음은 층이 바뀌어도 이어짐. 이미 같은 곡이 흐르면 다시 시작하지 않음.
	AudioSystem::Get().PlayLoop(AudioSystem::SoundId::MainTheme);

	// 맵에 적힌 좌표마다 좀비를 놓는 일은 ZombieSpawner가 맡음.
	zombieSpawner.SpawnMapZombies(*this);
}

std::vector<std::shared_ptr<Zombie>> FloorLevel::GetActiveZombies() const
{
	std::vector<std::shared_ptr<Zombie>> zombies;

	for (const std::shared_ptr<Actor>& actor : actorList)
	{
		std::shared_ptr<Zombie> zombie = Cast<Zombie>(actor);

		if (zombie && zombie->IsActive())
		{
			zombies.emplace_back(zombie);
		}
	}

	return zombies;
}

void FloorLevel::Tick(float deltaTime)
{
	if (Input::Get().GetKeyDown(VK_ESCAPE))
	{
		Engine::Get().Quit();
		return;
	}

	if (Input::Get().GetKeyDown(VK_F1))
	{
		// true와 false를 뒤집음.
		// F1을 누를 때마다 디버그 모드가 켜지고 꺼짐.
		runState->debugMode = !runState->debugMode;
	}

	if (Input::Get().GetKeyDown('M'))
	{
		showMap = !showMap;
	}

	// 맵을 보는 동안에는 플레이어와 좀비를 잠깐 멈춤.
	if (showMap)
	{
		return;
	}

	// Q/E를 누르는 동안 카메라만 회전한다.
	// 월드 좌표와 충돌 판정은 그대로라서 맵 로직에는 영향을 주지 않음.
	const float cameraRotationSpeed = 1.8f;

	if (Input::Get().GetKey('Q'))
	{
		isometricRenderer.RotateCamera(-cameraRotationSpeed * deltaTime);
	}

	if (Input::Get().GetKey('E'))
	{
		isometricRenderer.RotateCamera(cameraRotationSpeed * deltaTime);
	}

	// 잔상은 게임오버 상태에서도 계속 걷혀야 하므로 아래 분기보다 먼저 처리함.
	if (runState->transitionTimer > 0.0f)
	{
		runState->transitionTimer -= deltaTime;

		if (runState->transitionTimer <= 0.0f)
		{
			runState->transitionTimer = 0.0f;
			// 다 지워졌으면 복사본을 버려 메모리를 쥐고 있지 않게 함.
			runState->transitionFrame.clear();
		}
	}

	// 죽거나 탈출하면 Actor 업데이트를 멈추고 결과 화면으로 넘어감.
	// 재시작은 EndingLevel이 맡으므로 여기서는 R을 받지 않음.
	if (runState->isGameOver || runState->isCleared)
	{
		// 화면을 그대로 멈춰 두고 잠깐 보여준 뒤 넘김.
		endingDelayTimer += deltaTime;

		if (endingDelayTimer >= endingDelay)
		{
			Engine::Get().AddNewLevel<EndingLevel>(runState);
		}

		return;
	}
	// 플레이 중에만 H로 회복약 사용.
	if (Input::Get().GetKeyDown('H'))
	{
		UseMedicine();
	}

	// 출구 진입 여부를 먼저 결정해야 같은 프레임에 도착한 좀비 기록이 사라지지 않음.
	Level::Tick(deltaTime);
	if (!floorTransitionRequested && !runState->isGameOver && !runState->isCleared)
	{
		zombieSpawner.Update(deltaTime, *this, *runState);
	}

	AudioSystem::Get().Update(deltaTime);

	// 액터가 다 움직인 뒤에 시야를 갱신해야 한 프레임 늦지 않음.
	UpdateVisibility(deltaTime);
	UpdateCameraTwist(deltaTime);
}

// virtual 함수는 한번도 안불려도 본문이 있어야 함.
// 선언만 하고 두면 링크 에러 뜸.
void FloorLevel::Draw()
{
	float mapWidth = static_cast<float>(GetWidth());
	float mapHeight = static_cast<float>(GetHeight());

	// 타일을 키워서 맵이 화면보다 커졌으므로 카메라가 플레이어를 따라감.
	// 맵 가장자리에서는 멈춰서 바깥의 빈 공간을 비추지 않음.
	// 플레이어의 그리기용 좌표를 따라감.
	// 실제 타일 좌표를 쓰면 카메라도 칸 단위로 튀어서 화면이 덜컹거림.
	Vector2 playerPosition = GetPlayerPosition();
	float cameraTargetX = static_cast<float>(playerPosition.x) + 0.5f;
	float cameraTargetY = static_cast<float>(playerPosition.y) + 0.5f;

	if (std::shared_ptr<Player> currentPlayer = player.lock())
	{
		cameraTargetX = currentPlayer->GetVisualX() + 0.5f;
		cameraTargetY = currentPlayer->GetVisualY() + 0.5f;
	}

	isometricRenderer.SetCameraFollow(
		cameraTargetX, cameraTargetY, mapWidth, mapHeight);

	// 바닥은 통짜 사각형이 아니라 칸 단위로 그림.
	// 시야에 따라 칸마다 밝기가 달라서 한 번에 칠할 수 없음.
	//
	// 카메라가 돌아가면 y만으로는 앞뒤 순서를 정할 수 없음.
	// 화면에서 먼 칸부터 그려야 가까운 벽이 자연스럽게 앞을 가림.
	std::vector<Vector2> drawOrder;
	drawOrder.reserve(GetWidth() * GetHeight());

	for (int y = 0; y < GetHeight(); ++y)
	{
		for (int x = 0; x < GetWidth(); ++x)
		{
			drawOrder.emplace_back(x, y);
		}
	}

	std::stable_sort(
		drawOrder.begin(), drawOrder.end(),
		[&](const Vector2& left, const Vector2& right)
		{
			const int leftDepth = isometricRenderer.WorldToScreen(
				static_cast<float>(left.x) + 0.5f,
				static_cast<float>(left.y) + 0.5f).y;
			const int rightDepth = isometricRenderer.WorldToScreen(
				static_cast<float>(right.x) + 0.5f,
				static_cast<float>(right.y) + 0.5f).y;

			return leftDepth < rightDepth;
		});

	for (const Vector2& tilePosition : drawOrder)
	{
		const int x = tilePosition.x;
		const int y = tilePosition.y;
		TileType tile = GetTile(x, y);

			// 아직 한 번도 못 본 칸은 아예 그리지 않아 검게 남음.
			const int lightIndex = y * GetWidth() + x;
			const float light =
				lightIndex < static_cast<int>(tileLight.size())
				? tileLight[lightIndex] : 1.0f;

			Color floorShade = Color::FloorGrime;

			if (!ShadeByLight(light, Color::FloorGrime, floorShade))
			{
				continue;
			}

			isometricRenderer.DrawTile(
				static_cast<float>(x), static_cast<float>(y),
				floorShade, 0);

			// 핏자국은 바닥 위, 벽 아래에 깔림.
			// 많이 쌓일수록 안쪽 여백이 줄어 넓게 번진 것처럼 보임.
			if (lightIndex < static_cast<int>(bloodLevel.size())
				&& bloodLevel[lightIndex] > 0)
			{
				Vector2 bloodTopLeft = isometricRenderer.WorldToScreen(
					static_cast<float>(x), static_cast<float>(y));
				Vector2 bloodBottomRight = isometricRenderer.WorldToScreen(
					static_cast<float>(x) + 1.0f,
					static_cast<float>(y) + 1.0f);

				const int level = bloodLevel[lightIndex];
				const int insetX =
					(bloodBottomRight.x - bloodTopLeft.x) * (4 - level) / 10;
				const int insetY =
					(bloodBottomRight.y - bloodTopLeft.y) * (4 - level) / 10;

				isometricRenderer.DrawRect(
					bloodTopLeft.x + insetX, bloodTopLeft.y + insetY,
					bloodBottomRight.x - insetX, bloodBottomRight.y - insetY,
					light >= 0.72f ? Color::Danger : DimOnce(Color::Danger),
					0);
			}

			// 지금 눈에 보이는 칸만 원래 색, 기억에 남은 칸은 한 단계 어둡게.
			const bool bright = light >= 0.72f;

			if (tile == TileType::Wall || tile == TileType::ClosedDoor)
			{
				// 윗면을 자기 칸에 그리고 그 아래로 어두운 면을 조금 흘려서
				// 두께가 있는 덩어리처럼 보이게 함.
				// 세로로 이어진 벽은 아래 칸이 나중에 그려지며 앞면을 덮으므로
				// 줄 맨 아래에만 그림자가 남아 입체로 읽힘.
				Vector2 blockTopLeft = isometricRenderer.WorldToScreen(
					static_cast<float>(x), static_cast<float>(y));
				Vector2 blockBottomRight = isometricRenderer.WorldToScreen(
					static_cast<float>(x) + 1.0f,
					static_cast<float>(y) + 1.0f);

				// Subtle depth: two logical pixels for a 16-pixel tile.
				int blockDepth = std::abs(
					isometricRenderer.WorldToScreen(
						static_cast<float>(x), static_cast<float>(y) + 1.0f).y
					- isometricRenderer.WorldToScreen(
						static_cast<float>(x), static_cast<float>(y)).y) / 8;

				if (blockDepth < 1)
				{
					blockDepth = 1;
				}

				const bool isDoor = tile == TileType::ClosedDoor;
				const auto isSolid = [](TileType value)
				{
					return value == TileType::Wall || value == TileType::ClosedDoor;
				};

				isometricRenderer.DrawBox(
					static_cast<float>(x),
					static_cast<float>(y),
					1.0f, 1.0f, blockDepth,
					Shade(bright,
						isDoor ? Color::DoorTop : Color::FloorBase),
					Shade(bright,
						isDoor ? Color::DoorFront : Color::Shadow),
					!isSolid(GetTile(x, y + 1)),
					!isSolid(GetTile(x + 1, y)));
			}
			else if (tile == TileType::OpenDoor)
			{
				// 열린 문은 지나갈 수 있으므로 양옆 문설주만 남기고 가운데는 비움.
				Vector2 doorTopLeft =
					isometricRenderer.WorldToScreen(
						static_cast<float>(x),
						static_cast<float>(y));
				Vector2 doorBottomRight =
					isometricRenderer.WorldToScreen(
						static_cast<float>(x) + 1.0f,
						static_cast<float>(y) + 1.0f);

				// 문설주 두께도 타일 크기에 비례하게.
				int postWidth = (doorBottomRight.x - doorTopLeft.x) / 8;
				int postHeight = (doorBottomRight.y - doorTopLeft.y) / 8;

				if (postWidth < 1)
				{
					postWidth = 1;
				}

				if (postHeight < 1)
				{
					postHeight = 1;
				}

				// 문이 어느 벽에 붙어 있는지 보고 문설주 방향을 정함.
				// 좌우가 벽이면 가로벽에 난 문이라 문설주도 좌우에 있어야 함.
				// 방향을 안 맞추면 문이 벽을 가로지르는 것처럼 보여서 헷갈림.
				const bool postsOnSides =
					GetTile(x - 1, y) == TileType::Wall
					&& GetTile(x + 1, y) == TileType::Wall;

				const Color postColor = Shade(bright, Color::DoorEdge);

				if (postsOnSides)
				{
					isometricRenderer.DrawRect(
						doorTopLeft.x, doorTopLeft.y,
						doorTopLeft.x + postWidth, doorBottomRight.y,
						postColor, 1);

					isometricRenderer.DrawRect(
						doorBottomRight.x - postWidth, doorTopLeft.y,
						doorBottomRight.x, doorBottomRight.y,
						postColor, 1);
				}
				else
				{
					isometricRenderer.DrawRect(
						doorTopLeft.x, doorTopLeft.y,
						doorBottomRight.x, doorTopLeft.y + postHeight,
						postColor, 1);

					isometricRenderer.DrawRect(
						doorTopLeft.x, doorBottomRight.y - postHeight,
						doorBottomRight.x, doorBottomRight.y,
						postColor, 1);
				}
			}
			else if (tile == TileType::Exit)
			{
				// 출구는 올라가는 곳이라 높이 없이 타일 색만 바꿈.
				isometricRenderer.DrawTile(
					static_cast<float>(x),
					static_cast<float>(y),
					Shade(bright, Color::Stairs));
			}
		}

	for (const Vector2& medicinePosition : medicinePositions)
	{
		// 아직 못 본 곳의 약은 안 보여야 함. 벽 너머가 훤히 보이면 탐색할 이유가 없음.
		const int medicineIndex =
			medicinePosition.y * GetWidth() + medicinePosition.x;

		if (medicineIndex < static_cast<int>(tileLight.size())
			&& tileLight[medicineIndex] < 0.06f)
		{
			continue;
		}

		// 타일 안쪽에 여백을 두고 채움.
		// 타일 크기가 창에 따라 달라지므로 고정 칸 수가 아니라 비율로 잡음.
		Vector2 itemTopLeft = isometricRenderer.WorldToScreen(
			static_cast<float>(medicinePosition.x),
			static_cast<float>(medicinePosition.y));
		Vector2 itemBottomRight = isometricRenderer.WorldToScreen(
			static_cast<float>(medicinePosition.x) + 1.0f,
			static_cast<float>(medicinePosition.y) + 1.0f);

		const int insetX = (itemBottomRight.x - itemTopLeft.x) / 4;
		const int insetY = (itemBottomRight.y - itemTopLeft.y) / 4;

		isometricRenderer.DrawRect(
			itemTopLeft.x + insetX, itemTopLeft.y + insetY,
			itemBottomRight.x - insetX, itemBottomRight.y - insetY,
			Color::Skin, 1);
	}

	if (runState->debugMode)
	{
		Renderer::Get().SubmitCells(
			"DEBUG ON",
			Vector2(1, 3),
			Color::Skin,
			11);

		if (lastNoiseRange > 0)
		{
			Renderer::Get().Submit(
				"!", WorldToScreen(lastNoisePosition), Color::Danger, 3);

			Renderer::Get().SubmitCells(
				"Noise: " + std::to_string(lastNoiseRange),
				Vector2(1, 8), Color::Skin, 11);
		}
	}
	
	// 잔상은 액터보다 위, HUD보다 아래에 그림.
	DrawFloorTransition();

	GameHUD::Draw(*runState);
	Level::Draw();

	if (showMap)
	{
		DrawMapOverlay();
	}
}

void FloorLevel::UpdateCameraTwist(float deltaTime)
{
	const Vector2 playerPosition = GetPlayerPosition();

	if (GetWidth() <= 0 || GetHeight() <= 0)
	{
		return;
	}

	// 맵을 4개의 큰 구역으로 나눔. 작은 맵에서도 구역 크기가 1이 되지 않게 함.
	const int zoneWidth = (std::max)(1, GetWidth() / 4);
	const int zoneHeight = (std::max)(1, GetHeight() / 2);
	const int zoneX = (std::min)(3, playerPosition.x / zoneWidth);
	const int zoneY = (std::min)(1, playerPosition.y / zoneHeight);
	const int zone = zoneY * 4 + zoneX;

	if (zone != cameraTwistZone)
	{
		cameraTwistZone = zone;
		// 대각선으로 인접한 공간마다 반대 방향을 사용해 단조로움을 줄임.
		cameraTwistTarget = ((zoneX + zoneY) % 2 == 0)
			? 0.14f : -0.14f;
		cameraTwistTimer = 0.35f;
	}

	// Q/E를 누르는 동안은 수동 회전을 우선함.
	const bool manualRotation =
		Input::Get().GetKey('Q') || Input::Get().GetKey('E');

	if (manualRotation)
	{
		cameraTwistTimer = 0.0f;
		return;
	}

	if (cameraTwistTimer <= 0.0f)
	{
		return;
	}

	// 0.35초 동안 목표 각도까지 부드럽게 이동.
	const float remaining = (std::max)(cameraTwistTimer, 0.001f);
	const float maxStep =
		std::abs(cameraTwistTarget - isometricRenderer.GetCameraAngle())
		* deltaTime / remaining;
	isometricRenderer.SmoothRotateTo(cameraTwistTarget, maxStep);
	cameraTwistTimer -= deltaTime;

	if (cameraTwistTimer < 0.0f)
	{
		cameraTwistTimer = 0.0f;
	}
}

void FloorLevel::DrawFloorTransition()
{
	if (runState->transitionTimer <= 0.0f
		|| runState->transitionFrame.empty())
	{
		return;
	}

	const int width = Engine::Get().GetWidth();
	const int height = Engine::Get().GetHeight();

	// 화면 크기가 바뀐 뒤라면 복사본을 믿을 수 없으므로 그리지 않음.
	if (static_cast<int>(runState->transitionFrame.size()) < width * height)
	{
		return;
	}

	// 0에서 1로 감. 클수록 더 어두워지고 더 많이 지워짐.
	float progress = 1.0f
		- runState->transitionTimer / runState->transitionDuration;

	if (progress < 0.0f)
	{
		progress = 0.0f;
	}

	if (progress > 1.0f)
	{
		progress = 1.0f;
	}

	// 팔레트 색을 한 단계 어둡게 바꾸는 표.
	// 여러 번 통과시키면 결국 Ink(0)로 수렴함.
	static const int dimTable[16] =
	{
		0,   // Ink
		0,   // WallFront
		1,   // FloorBase
		2,   // FloorGrime
		3,   // ClothDark
		4,   // ClothLight
		5,   // WallTop
		9,   // DoorTop
		7,   // DoorEdge
		14,  // DoorFront
		9,   // RotSkin
		10,  // Stairs
		1,   // Danger
		4,   // Skin
		0,   // Shadow
		1    // Hair
	};

	// 4x4 순서 디더. 칸마다 사라지는 시점이 달라서 흩어지듯 지워짐.
	static const int ditherTable[4][4] =
	{
		{  0,  8,  2, 10 },
		{ 12,  4, 14,  6 },
		{  3, 11,  1,  9 },
		{ 15,  7, 13,  5 }
	};

	const int dimCount = 1 + static_cast<int>(progress * 3.0f);
	const float eraseThreshold = progress * 16.0f;

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			// 지워질 차례가 지난 칸은 건너뜀.
			if (ditherTable[y % 4][x % 4] < eraseThreshold)
			{
				continue;
			}

			const CHAR_INFO& cell =
				runState->transitionFrame[y * width + x];

			// 이 게임은 배경색으로 그림을 그리므로 배경색만 가져옴.
			int colorIndex = (cell.Attributes >> 4) & 0x0F;

			for (int step = 0; step < dimCount; ++step)
			{
				colorIndex = dimTable[colorIndex];
			}

			// 완전히 어두워진 칸은 그릴 필요가 없음.
			if (colorIndex == 0)
			{
				continue;
			}

			Renderer::Get().Submit(
				" ",
				Vector2(x, y),
				static_cast<Color>(colorIndex << 4),
				9);
		}
	}
}

bool FloorLevel::LoadMap(const std::string& filename)
{
	MapData data;

	// 맵 읽기에 실패하면 객체를 만들지 않고 종료.
	if (!MapLoader::Load(filename, data))
	{
		return false;
	}

	// 성공 시 읽은 지형과 배치 정보만 적용.
	// 기존 코드가 플레이어와 좀비 생성.
	tileMap.SetData(data.width, data.height, std::move(data.tiles));
	// 시야·기억 배열도 같은 맵 크기를 사용하므로 함께 갱신해야 함.
	width = data.width;
	height = data.height;
	// 지형 설정 후 그 지형에서 문을 찾아 피격 횟수를 준비함.
	// 새 층을 읽을 때만 초기화되므로 문을 열고 닫아도 누적 피해는 유지됨.
	doorSystem.Initialize(tileMap);

	playerStart = data.playerStart;
	zombieStarts = std::move(data.zombieStarts);
	medicinePositions = std::move(data.medicinePositions);
	return true;
}

// 그냥 배열을 읽으면 맵 밖 좌표에서 프로그램이 죽음.
// TileType FloorLevel::GetTile(int x, int y) const
// {
// 	// Wall을 돌려준다. 맵 밖은 벽으로 취급.
// 	// isWalkable이 자동으로 false.
// 	if (x < 0 || x >= width || y < 0 || y >= height)
// 	{
// 		return TileType::Wall;
// 	}
// 
// 	return tiles[(y * width) + x];
// }
TileType FloorLevel::GetTile(int x, int y) const
{
	return tileMap.GetTile(x, y);
}

// 경계검사까지 해주는 IsWalkable.
bool FloorLevel::IsWalkable(const Vector2& position) const
{
	TileType tile = GetTile(position.x, position.y);
	return tile != TileType::Wall && tile != TileType::ClosedDoor;
}

Vector2 FloorLevel::GetPlayerPosition() const
{
	// lock()은 Player가 아직 살아 있는지 확인하고 잠깐 사용할 수 있게 해주는 함수.
	// 살아 있으면 현재 위치를 반환, 이미 사라지면 안전하게 시작 위치 반환.
	std::shared_ptr<Player> foundPlayer = player.lock();

	if (!foundPlayer)
	{
		return playerStart;
	}

	return foundPlayer->GetPosition();
}

bool FloorLevel::HasLineOfSight(
	const Vector2& from,
	const Vector2& to) const
{
	// 현재 검사 중인 칸.
	int x = from.x;
	int y = from.y;

	// 두 위치의 거리.
	int deltaX = std::abs(to.x - from.x);
	int deltaY = std::abs(to.y - from.y);

	// 좌표가 움직일 방향.
	int stepX = from.x < to.x ? 1 : -1;
	int stepY = from.y < to.y ? 1 : -1;

	// 실제 직선에 가깝게 가려면 x와 y 중 어느 쪽을 움직일지 정하는 값.
	int error = deltaX - deltaY;

	while (x != to.x || y != to.y)
	{
		int twiceError = error * 2;

		if (twiceError > -deltaY)
		{
			error -= deltaY;
			x += stepX;
		}

		if (twiceError < deltaX)
		{
			error += deltaX;
			y += stepY;
		}

		// 지나가는 칸에서 벽을 만나면 시야가 막혔다고 반환함.
		// 닫힌 문은 이동과 시야를 막고, 열린 문은 둘 다 통과시킴.
		// A*도 IsWalkable을 사용하므로 닫힌 문을 통과하는 경로는 선택하지 않게 함.
		TileType tile = GetTile(x, y);
		if (tile == TileType::Wall || tile == TileType::ClosedDoor)
		{
			return false;
		}
	}

	return true;
}

// 약 하나로 체력 30을 회복하고, 최대 체력은 100.
bool FloorLevel::UseMedicine()
{
	if (runState->isGameOver || runState->isCleared
		|| runState->medicineCount <= 0 || runState->health >= 100)
	{
		return false;
	}

	--runState->medicineCount;
	runState->health += 30;

	if (runState->health > 100)
	{
		runState->health = 100;
	}

	return true;
}

void FloorLevel::PickUpMedicineAt(const Vector2& position)
{
	for (auto iter = medicinePositions.begin();
		iter != medicinePositions.end(); ++iter)
	{
		if (*iter == position)
		{
			++runState->medicineCount;
			medicinePositions.erase(iter);
			return;
		}
	}
}

void FloorLevel::ToggleDoorAt(const Vector2& position)
{
	if (runState->isGameOver || runState->isCleared)
	{
		return;
	}

	TileType tile = GetTile(position.x, position.y);

	if (tile == TileType::ClosedDoor)
	{
		tileMap.SetTile(position.x, position.y, TileType::OpenDoor);
		lastVisibilityOrigin = Vector2(-1, -1);
		// 거리는 가로, 세로 이동 칸수의 합.
		MakeNoise(position, 6);
	}
	else if (tile == TileType::OpenDoor)
	{
		if (HasZombieAt(position) || GetPlayerPosition() == position)
		{
			return;
		}

		tileMap.SetTile(position.x, position.y, TileType::ClosedDoor);
		lastVisibilityOrigin = Vector2(-1, -1);
	}
}

void FloorLevel::MakeNoise(const Vector2& noisePosition, int range)
{
	lastNoisePosition = noisePosition;
	lastNoiseRange = range;

	for (const std::shared_ptr<Actor>& actor : actorList)
	{
		std::shared_ptr<Zombie> zombie = Cast<Zombie>(actor);

		if (!zombie || !zombie->IsActive())
		{
			continue;
		}

		Vector2 zombiePosition = zombie->GetPosition();
		int distance =
			std::abs(zombiePosition.x - noisePosition.x)
			+ std::abs(zombiePosition.y - noisePosition.y);

		if (distance <= range)
		{
			zombie->HearNoise(noisePosition);
		}
	}
}

// 좀비가 FloorLevel에 요청, 실제 피격 횟수와 파괴 처리는 DoorSystem이 담당.
bool FloorLevel::HitDoorAt(const Vector2& position)
{
	if (runState->isGameOver || runState->isCleared)
	{
		return false;
	}

	const bool destroyed = doorSystem.HitDoor(tileMap, position);
	if (destroyed)
	{
		lastVisibilityOrigin = Vector2(-1, -1);
	}
	return destroyed;
}

// 플레이어가 출구에 도착했을 때 호출할 예정.
// 닫힌 문에 완전히 갇힌 좀비는 제외함.
// 이전 층에서 문을 부수고 뒤늦게 올라오는 처리는 아직 구현하지 않는 범위.
// 등장 지연은 ZombieData 기본값인 3초 사용.


bool FloorLevel::FindSpawnPosition(
	const Vector2& start, Vector2& outPosition) const
{
	if (!IsWalkable(start))
	{
		return false;
	}

	// positions는 검사할 좌표 대기열.
	std::queue<Vector2> positions;
	// visited는 이미 대기열에 넣은 칸의 기록.
	// 시작점을 먼저 넣고, 같은 칸을 반복해서 넣지 않도록 방문 표시를 함.
	std::vector<bool> visited(GetWidth() * GetHeight(), false);

	positions.push(start);
	visited[start.y * GetWidth() + start.x] = true;

	const Vector2 directions[] =
	{
		Vector2(0, -1),
		Vector2(0, 1),
		Vector2(-1, 0),
		Vector2(1, 0)
	};

	// 대기열 맨 앞 좌표를 꺼냄. 
	while (!positions.empty())
	{
		Vector2 current = positions.front();
		positions.pop();

		//플레이어나 좀비가 없으면 그 위치 반환.
		if (current != GetPlayerPosition() && !HasZombieAt(current))
		{
			outPosition = current;
			return true;
		}

		// 점유된 칸이면 이동 가능한 이웃을 대기열 뒤에 추가하므로 가까운 칸부터 검사함.
		for (const Vector2& direction : directions)
		{
			Vector2 next = current + direction;

			if (!IsWalkable(next))
			{
				continue;
			}

			int index = next.y * GetWidth() + next.x;

			if (visited[index])
			{
				continue;
			}

			visited[index] = true;
			positions.push(next);
		}
	}
	// 모두 검사해도 빈칸이 없으면 false.
	return false;
}

// 도착한 좀비는 입구 위치를 조사하고, 플레이어를 직접 발견하면 추격.
// 현재 플레이어 위치를 자동으로 알려주지는 않음.
// 생성한 기록은 삭제하고 바로 돌아가므로 한 프레임에 한 마리만 추가.

bool FloorLevel::IsTileVisibleFrom(
	const Vector2& from, int toX, int toY) const
{
	int x = from.x;
	int y = from.y;

	int deltaX = std::abs(toX - x);
	int deltaY = std::abs(toY - y);

	int stepX = x < toX ? 1 : -1;
	int stepY = y < toY ? 1 : -1;

	int error = deltaX - deltaY;

	while (x != toX || y != toY)
	{
		int twiceError = error * 2;

		if (twiceError > -deltaY)
		{
			error -= deltaY;
			x += stepX;
		}

		if (twiceError < deltaX)
		{
			error += deltaX;
			y += stepY;
		}

		// 목표 칸 자체는 벽이어도 보임.
		// 벽이 안 보이면 방의 윤곽이 사라져서 지도가 읽히지 않음.
		if (x == toX && y == toY)
		{
			break;
		}

		TileType tile = GetTile(x, y);

		if (tile == TileType::Wall || tile == TileType::ClosedDoor)
		{
			return false;
		}
	}

	return true;
}

bool FloorLevel::IsTileLit(int x, int y) const
{
	if (x < 0 || y < 0 || x >= width || y >= height)
	{
		return false;
	}

	const int index = y * width + x;

	if (index < 0 || index >= static_cast<int>(tileVisibleNow.size()))
	{
		return false;
	}

	return tileVisibleNow[index];
}

bool FloorLevel::IsTileSeen(int x, int y) const
{
	if (x < 0 || y < 0 || x >= GetWidth() || y >= GetHeight())
	{
		return false;
	}

	const int index = y * GetWidth() + x;
	return index >= 0 && index < static_cast<int>(tileSeen.size())
		&& tileSeen[index];
}

void FloorLevel::DrawMapOverlay()
{
	const int mapWidth = GetWidth();
	const int mapHeight = GetHeight();
	// 한 타일을 콘솔에서 2열 x 2행으로 확대해 작은 문자 지도보다
	// 픽셀 미니맵처럼 읽히게 함. Floor1(64x24)도 130x50 안에 들어감.
	const int mapScaleX = 2;
	const int mapScaleY = 2;
	const int mapPixelWidth = mapWidth * mapScaleX;
	const int mapPixelHeight = mapHeight * mapScaleY;
	const int panelWidth = mapPixelWidth + 2;
	const int panelHeight = mapPixelHeight + 2;
	const int panelX = (Engine::Get().GetWidth() - panelWidth) / 2;
	const int consoleHeight = Engine::Get().GetConsoleHeight();
	const int panelY = (consoleHeight - panelHeight) / 2;
	const int panelOrder = 20;

	const Color backdropColor = static_cast<Color>(
		static_cast<WORD>(Color::Ink) << 4);
	const Color panelColor = static_cast<Color>(
		static_cast<WORD>(Color::WallFront) << 4);

	// 월드와 HUD를 먼저 어둡게 덮어 지도만 읽히게 함.
	const std::string backdropRow(Engine::Get().GetWidth(), ' ');
	for (int row = 0; row < consoleHeight; ++row)
	{
		Renderer::Get().SubmitCells(
			backdropRow,
			Vector2(0, row),
			backdropColor,
			panelOrder);
	}

	const std::string blankRow(panelWidth, ' ');
	for (int row = 0; row < panelHeight; ++row)
	{
		Renderer::Get().SubmitCells(
			blankRow,
			Vector2(panelX, panelY + row),
			panelColor,
			panelOrder);
	}

	Renderer::Get().SubmitCells(
		"MAP  FLOOR " + std::to_string(runState->currentFloor)
			+ "  [M] CLOSE",
		Vector2(panelX, panelY - 3),
		Color::WallTop,
		panelOrder + 1);

	// 타일 색으로 전체 맵을 표시. 시야 기억과 관계없이 구조는 항상 보임.
	auto drawMapTile = [&](int x, int y, Color baseColor, int order)
	{
		const Color fillColor = static_cast<Color>(
			static_cast<WORD>(baseColor) << 4);
		const Vector2 tilePosition(
			panelX + 1 + x * mapScaleX,
			panelY + 1 + y * mapScaleY);

		for (int row = 0; row < mapScaleY; ++row)
		{
			Renderer::Get().SubmitCells(
				"  ",
				Vector2(tilePosition.x, tilePosition.y + row),
				fillColor,
				order);
		}
	};

	for (int y = 0; y < mapHeight; ++y)
	{
		for (int x = 0; x < mapWidth; ++x)
		{
			const TileType tile = GetTile(x, y);
			Color color = Color::FloorBase;

			if (tile == TileType::Wall)
			{
				color = Color::WallFront;
			}
			else if (tile == TileType::ClosedDoor)
			{
				color = Color::DoorFront;
			}
			else if (tile == TileType::OpenDoor)
			{
				color = Color::DoorTop;
			}
			else if (tile == TileType::Exit)
			{
				color = Color::Stairs;
			}

			drawMapTile(x, y, color, panelOrder + 1);
		}
	}

	// 회복약은 실제로 발견한 위치만 표시해 탐색 정보는 유지.
	for (const Vector2& medicinePosition : medicinePositions)
	{
		if (!IsTileSeen(medicinePosition.x, medicinePosition.y))
		{
			continue;
		}

		drawMapTile(
			medicinePosition.x,
			medicinePosition.y,
			Color::Skin,
			panelOrder + 2);
		Renderer::Get().SubmitCells(
			"+",
			Vector2(panelX + 1 + medicinePosition.x * mapScaleX,
				panelY + 1 + medicinePosition.y * mapScaleY),
			static_cast<Color>(
				static_cast<WORD>(Color::Ink)
				| (static_cast<WORD>(Color::Skin) << 4)),
			panelOrder + 2);
	}

	const Vector2 playerPosition = GetPlayerPosition();
	// 현재 위치는 항상 빨간 타일로 강조.
	drawMapTile(
		playerPosition.x,
		playerPosition.y,
		Color::Danger,
		panelOrder + 3);

	const Color playerMarkerColor = static_cast<Color>(
		static_cast<WORD>(Color::WallTop)
		| (static_cast<WORD>(Color::Danger) << 4));
	Renderer::Get().SubmitCells(
		"P",
		Vector2(panelX + 1 + playerPosition.x * mapScaleX,
			panelY + 1 + playerPosition.y * mapScaleY),
		playerMarkerColor,
		panelOrder + 3);

	for (const std::shared_ptr<Zombie>& zombie : GetActiveZombies())
	{
		const Vector2 zombiePosition = zombie->GetPosition();

		if (!IsTileLit(zombiePosition.x, zombiePosition.y))
		{
			continue;
		}

		// 보이는 좀비만 지도 위에 표시. 맵 구조와 적 위치 정보는 분리함.
		drawMapTile(
			zombiePosition.x,
			zombiePosition.y,
			Color::Danger,
			panelOrder + 3);
		Renderer::Get().SubmitCells(
			"Z",
			Vector2(panelX + 1 + zombiePosition.x * mapScaleX,
				panelY + 1 + zombiePosition.y * mapScaleY),
			static_cast<Color>(
				static_cast<WORD>(Color::WallTop)
				| (static_cast<WORD>(Color::Danger) << 4)),
			panelOrder + 3);
	}
}

void FloorLevel::UpdateVisibility(float deltaTime)
{
	const int cellCount = width * height;

	if (cellCount <= 0)
	{
		return;
	}

	if (static_cast<int>(tileLight.size()) != cellCount)
	{
		tileLight.assign(cellCount, 0.0f);
		tileSeen.assign(cellCount, false);
		tileVisibleNow.assign(cellCount, false);
		lastVisibilityOrigin = Vector2(-1, -1);
	}

	const Vector2 origin = GetPlayerPosition();

	// 시야를 다시 긋는 건 플레이어가 칸을 옮겼을 때만.
	// 매 프레임 다시 그으면 낭비고, 결과도 어차피 같음.
	if (origin != lastVisibilityOrigin)
	{
		lastVisibilityOrigin = origin;

		std::fill(tileVisibleNow.begin(), tileVisibleNow.end(), false);

		for (int y = origin.y - sightRadius; y <= origin.y + sightRadius; ++y)
		{
			for (int x = origin.x - sightRadius; x <= origin.x + sightRadius; ++x)
			{
				if (x < 0 || y < 0 || x >= width || y >= height)
				{
					continue;
				}

				// 사각형이 아니라 원 모양으로 잘라야 손전등처럼 보임.
				const int dx = x - origin.x;
				const int dy = y - origin.y;

				if (dx * dx + dy * dy > sightRadius * sightRadius)
				{
					continue;
				}

				if (!IsTileVisibleFrom(origin, x, y))
				{
					continue;
				}

				const int index = y * width + x;
				tileVisibleNow[index] = true;
				// 한 번 본 칸은 계속 기억함.
				tileSeen[index] = true;
			}
		}
	}

	// 밝기는 매 프레임 목표치로 당겨옴. 시야가 뚝 끊기지 않고 스며들 듯 변함.
	float factor = lightFadeSpeed * deltaTime;

	if (factor > 1.0f)
	{
		factor = 1.0f;
	}

	for (int index = 0; index < cellCount; ++index)
	{
		float target = 0.0f;

		if (tileVisibleNow[index])
		{
			target = 1.0f;
		}
		else if (tileSeen[index])
		{
			target = rememberedLight;
		}

		tileLight[index] += (target - tileLight[index]) * factor;
	}
}

void FloorLevel::AddBlood(const Vector2& position, int amount)
{
	if (position.x < 0 || position.y < 0
		|| position.x >= width || position.y >= height)
	{
		return;
	}

	const int cellCount = width * height;

	if (static_cast<int>(bloodLevel.size()) != cellCount)
	{
		bloodLevel.assign(cellCount, 0);
	}

	const int index = position.y * width + position.x;

	int level = bloodLevel[index] + amount;

	// 너무 진해지면 바닥이 안 보이므로 상한을 둠.
	if (level > 3)
	{
		level = 3;
	}

	bloodLevel[index] = static_cast<unsigned char>(level);
}
