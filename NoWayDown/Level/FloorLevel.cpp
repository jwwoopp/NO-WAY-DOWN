#include "FloorLevel.h"
#include <Actor/Player.h>
#include <Actor/Zombie.h>

#include <Engine/Engine.h>
#include <Input/Input.h>
#include <Renderer/Renderer.h>
#include <Windows.h>
#include <iostream>
#include <cstdlib>

using namespace Craft;

FloorLevel::FloorLevel(const std::shared_ptr<RunState>& newRunState)
	// 전달받은 shared_ptr를 멤버 변수 runState에 저장하는 생성자 초기화 목록.
	// 두 shared_ptr이 같은 RunState를 가리키므로
	// Level이 바뀌어도 동일한 진행데이터를 넘길 수 있다.
	: runState(newRunState)
{
}

void FloorLevel::MoveToNextFloor()
{	
	// 4층 출구에 도착하면 클리어를 기록하고 5층 생성을 요청하지 않음.
	if (runState->currentFloor >= 4)
	{
		runState->isCleared = true;
		return;
	}
	++runState->currentFloor;
	Engine::Get().AddNewLevel<FloorLevel>(runState);
}

void FloorLevel::DamagePlayer(int amount)
{

	// 탈출한 프레임에 좀비의 공격 처리가 남아있어도 피해를 받지 않게 함.
	if (runState->isGameOver || runState->isCleared)
	{
		return;
	}

	runState->health -= amount;

	if (runState->health <= 0)
	{
		runState->health = 0;
		runState->isGameOver = true;
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
	std::string mapFilename = "../Assets/Floor" + std::to_string(runState->currentFloor) + ".txt";
	LoadMap(mapFilename);
	// 그 좌표에 플레이어를 생성함.
	// SpawnActor<Player>(playerStart);
	player = SpawnActor<Player>(playerStart);
	
	// 좀비 생성.
	for (const Vector2& zombieStart : zombieStarts)
	{
		SpawnActor<Zombie>(zombieStart);
	}
}

void FloorLevel::Tick(float deltaTime)
{
	if (Input::Get().GetKeyDown(VK_F1))
	{
		// true와 false를 뒤집음.
		// F1을 누를 때마다 디버그 모드가 켜지고 꺼짐.
		runState->debugMode = !runState->debugMode;
	}

	// 클리어 때도 Actor 업데이트가 멈추고 R 입력을 받음.
	if (runState->isGameOver || runState->isCleared)
	{
		if (Input::Get().GetKeyDown('R'))
		{
			runState->health = 100;
			runState->currentFloor = 1;
			runState->isGameOver = false;
			runState->isCleared = false;
			Engine::Get().AddNewLevel<FloorLevel>(runState);
		}

		return;
	}

	// Player와 Zombie가 계속 등장하도록 호출.
	Level::Tick(deltaTime);
}

// virtual 함수는 한번도 안불려도 본문이 있어야 함.
// 선언만 하고 두면 링크 에러 뜸.
void FloorLevel::Draw()
{
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			std::string image = ".";
			Color color = Color::Blue;

			switch (GetTile(x, y))
			{
			case TileType::Wall:
				image = "#";
				color = Color::White;
				break;

			case TileType::Exit:
				image = "E";
				color = Color::Yellow;
				break;

			default:
				break;
			}

			Renderer::Get().Submit(image, Vector2(x, y), color, 0);
		}
	}
	if (runState->debugMode)
	{
		Renderer::Get().Submit(
			"DEBUG ON",
			Vector2(22, 3),
			Color::Yellow,
			1);
	}
	
	Renderer::Get().Submit(
		"Floor: " + std::to_string(runState->currentFloor),
		Vector2(22, 1),
		Color::White,
		1
	);

	Renderer::Get().Submit(
		"HP: " + std::to_string(runState->health),
		Vector2(22, 2),
		Color::Red,
		1
	);

	if (runState->isGameOver || runState->isCleared)
	{
		Renderer::Get().Submit(
			runState->isCleared ? "ESCAPED!" : "GAME OVER",
			Vector2(22, 5),
			runState->isCleared ? Color::Green : Color::Red,
			10);

		Renderer::Get().Submit(
			"R: Restart", Vector2(22, 6), Color::White, 10);
	}

	Level::Draw();
}

void FloorLevel::LoadMap(const std::string& filename)
{
	FILE* file = nullptr;
	// fopen_s는 C함수라 옛날식 문자열 (const char*)만 받음.
	// 그래서 std::string 안의 글자들을 C가 이해하는 형태로 내주는 함수.
	fopen_s(&file, filename.c_str(), "rt");

	if (!file)
	{
		std::cout << "Failed to open map file.\n";
		__debugbreak();
		return;
	}

	// 파일 끝으로 이동해서 그 위치를 읽으면 그게 파일 크기.
	// 맨 끝으로 이동.
	fseek(file, 0, SEEK_END);
	// 지금 몇 번째인지.
	long fileSize = ftell(file);
	// 다시 맨 앞으로.
	rewind(file);

	// 빈 파일이면 더 진행할 수 없음.
	if (fileSize <= 0)
	{
		std::cout << "Map file is empty.\n";
		fclose(file);
		file = nullptr;
		return;
	}

	// 파일 크기만큼 버퍼 확보하고 읽기.
	// 파일 크기 만큼 상자를 만듦.
	// {} 로 전부 0으로 채움.
	// new는 힙에서 빌려오는 거라 크기를 그때그때 정할 수 있음.
	char* buffer = new char[fileSize] {};
	// 대신 이번엔 크기를 실행 중에 정함.
	// char buffer[fileSize] 안됨. 대괄호 안에는 미리 정해진 숫자만 들어갈 수 있음.
	size_t readSize = fread(buffer, sizeof(char), fileSize, file);

	// fread는 fileSize보다 많이 읽을 수 없지만, 분석기에게 명시.
	if (readSize > static_cast<size_t>(fileSize))
	{
		readSize = static_cast<size_t>(fileSize);
	}

	// 맵을 글자 하나씩 훑으면서 tiles를 채움.
	int x = 0;
	int y = 0;

	for (size_t index = 0; index < readSize; ++index)
	{
		char mapCharacter = buffer[index];

		// 줄바꿈을 만나면 x를 0으로 돌리고, y를 1 늘림.
		if (mapCharacter == '\n')
		{
			// 첫 줄의 길이를 맵 너비로 확정. 둘째 줄부터는 넘어감.
			if (width == 0)
			{
				width = x;
			}

			x = 0;
			++y;
			continue;
		}

		// 글자를 타일로 바꿈.
		switch (mapCharacter)
		{
		case '#':
			tiles.emplace_back(TileType::Wall);
			break;

		case 'e':
			tiles.emplace_back(TileType::Exit);
			break;

		case 'p':
			// 플레이어 자리는 지형상 바닥. 좌표만 따로 기억.
			playerStart = Vector2(x, y);
			tiles.emplace_back(TileType::Floor);
			break;

		case 'z':
			// z는 Actor가 놓일 위치일 뿐 지형은 바닥임.
			zombieStarts.emplace_back(x, y);
			tiles.emplace_back(TileType::Floor);
			break;

		default:
			// 예상 못한 글자가 와도 바닥으로 넘어가게 하려는 것.
			// 맵 파일에 오타가 있더라도 게임이 안 죽음.
			tiles.emplace_back(TileType::Floor);
			break;
		}

		++x;
	}

	// 마지막 줄에 개행이 없으면 width가 아직 0일 수 있음.
	if (width == 0)
	{
		width = x;
	}

	if (width > 0)
	{
		// 높이는 나눗셈으로 구함.
		// 칸 / 너비 = 높이(줄)
		// y를 세는 법도 있지만 마지막 줄에 개행이 없으면 그 줄이 안 세짐.
		height = static_cast<int>(tiles.size()) / width;
	}
	// 배열로 빌렸으니 배열로 돌려줘야함. - 메모리 leak 발생할 수 있음.
	delete[] buffer;
	buffer = nullptr;
	
	fclose(file);
	file = nullptr;
}

// 그냥 배열을 읽으면 맵 밖 좌표에서 프로그램이 죽음.
TileType FloorLevel::GetTile(int x, int y) const
{
	// Wall을 돌려준다. 맵 밖은 벽으로 취급.
	// isWalkable이 자동으로 false.
	if (x < 0 || x >= width || y < 0 || y >= height)
	{
		return TileType::Wall;
	}

	return tiles[(y * width) + x];
}

// 경계검사까지 해주는 IsWalkable.
bool FloorLevel::IsWalkable(const Vector2& position) const
{
	return GetTile(position.x, position.y) != TileType::Wall;
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
		if (GetTile(x, y) == TileType::Wall)
		{
			return false;
		}
	}

	return true;
}