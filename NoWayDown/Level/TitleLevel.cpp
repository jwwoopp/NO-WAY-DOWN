#include "TitleLevel.h"
#include "../Audio/AudioSystem.h"
#include "FloorLevel.h"
#include "../UI/BlockFont.h"
#include "../UI/SceneArt.h"

#include <Engine/Engine.h>
#include <Input/Input.h>
#include <Renderer/Renderer.h>

#include <Windows.h>
#include <string>

using namespace Craft;

namespace
{
	// 화면 가운데에 오도록 시작 x를 구함.
	int CenteredX(int contentWidth)
	{
		return (Engine::Get().GetWidth() - contentWidth) / 2;
	}

	// 콘솔 기본 크기 글자를 가운데 정렬해서 출력.
	void DrawCenteredText(
		const std::string& text, int y, Color color, int sortingOrder = 10)
	{
		Renderer::Get().SubmitCells(
			text,
			Vector2(CenteredX(static_cast<int>(text.size())), y),
			color,
			sortingOrder);
	}
}

TitleLevel::TitleLevel(const std::shared_ptr<RunState>& newRunState)
	: runState(newRunState)
{
}

void TitleLevel::StartNewRun()
{
	// 이전 판의 기록이 남아 있으면 새 게임이 오염되므로 전부 초기화함.
	runState->currentFloor = 1;
	runState->health = 100;
	runState->medicineCount = 0;
	runState->ammoCount = 6;
	runState->incomingZombies.clear();
	runState->isGameOver = false;
	runState->isCleared = false;

	// 이전 판의 잔상이 새 게임 첫 화면에 남지 않게 함.
	runState->transitionFrame.clear();
	runState->transitionTimer = 0.0f;

	Engine::Get().AddNewLevel<FloorLevel>(runState);
}

int TitleLevel::FloorLineY() const
{
	// SceneArt와 BlockFont는 SubmitCells를 쓰므로 좌표가 "셀" 기준임.
	// GetHeight()는 논리 높이(셀의 두 배)라 그대로 쓰면 화면 아래로 밀려남.
	// 실제로 그럴 수 있는 행 수는 GetConsoleHeight()임.
	//
	// 바닥선 아래에 안내를 넣을 자리를 남김.
	// 화면이 넉넉하면 블록 글꼴 세 줄(줄당 7행)을 쓰므로 더 위로 올리고,
	// 좁으면 작은 글자 세 줄만 들어가게 아래에 붙임.
	const int consoleHeight = Engine::Get().GetConsoleHeight();

	return consoleHeight >= 40 ? consoleHeight - 24 : consoleHeight - 4;
}

int TitleLevel::DoorX() const
{
	// 무대 오른쪽 끝에서 문 폭만큼 안쪽.
	return StageLeft() + StageWidth() - SceneArt::DoorWidth() - 6;
}

int TitleLevel::StageWidth() const
{
	return Engine::Get().GetWidth() / 2;
}

int TitleLevel::StageLeft() const
{
	return (Engine::Get().GetWidth() - StageWidth()) / 2;
}

bool TitleLevel::IsNearDoor() const
{
	const float doorCenter =
		static_cast<float>(DoorX() + SceneArt::DoorWidth() / 2);
	const float personCenter =
		personX + SceneArt::PersonWidth() * 0.5f;

	const float distance = personCenter - doorCenter;

	return distance > -11.0f && distance < 11.0f;
}

void TitleLevel::BeginDoorOpening()
{
	if (doorOpening)
	{
		return;
	}

	doorOpening = true;
	doorOpeningTimer = 0.0f;
}

void TitleLevel::Tick(float deltaTime)
{
	blinkTimer += deltaTime;

	if (doorOpening)
	{
		doorOpeningTimer += deltaTime;

		// 문이 열린 뒤 바로 장면을 바꾸지 않고 짧게 기다려 긴장감을 줌.
		if (doorOpeningTimer >= 0.55f)
		{
			StartNewRun();
		}

		Level::Tick(deltaTime);
		return;
	}

	if (personX < 0.0f)
	{
		// 문에서 조금 떨어진 곳에서 시작해 직접 걸어가게 함.
		personX = Engine::Get().GetWidth() / 2.0f - 18.0f;
	}

	if (Input::Get().GetKeyDown(VK_ESCAPE))
	{
		Engine::Get().Quit();
		return;
	}

	// 문 앞에서 F를 누르면 문을 열고 들어감.
	// 게임 안에서도 F가 문을 여는 키라 조작이 그대로 이어짐.
	if (IsNearDoor() && Input::Get().GetKeyDown('F'))
	{
		BeginDoorOpening();
		return;
	}

	// 문까지 걸어가기 어려운 사람을 위해 Enter도 그대로 받아둠.
	if (Input::Get().GetKeyDown(VK_RETURN))
	{
		BeginDoorOpening();
		return;
	}

	// 게임과 같은 키로 좌우 이동. 여기서 조작을 먼저 익히게 됨.
	float move = 0.0f;

	if (Input::Get().GetKey(VK_LEFT))
	{
		move -= 1.0f;
	}

	if (Input::Get().GetKey(VK_RIGHT))
	{
		move += 1.0f;
	}

	personX += move * walkSpeed * deltaTime;

	// 화면 밖으로 나가지 않게 가둠.
	const float maxX = static_cast<float>(
		Engine::Get().GetWidth() - SceneArt::PersonWidth() - 2);

	if (personX < 2.0f)
	{
		personX = 2.0f;
	}

	if (personX > maxX)
	{
		personX = maxX;
	}

	AudioSystem::Get().Update(deltaTime);
	Level::Tick(deltaTime);
}

void TitleLevel::Draw()
{
	const int screenWidth = Engine::Get().GetWidth();
	const int floorLineY = FloorLineY();
	const int standY = floorLineY - 1;
	const int doorX = DoorX();
	const int stageWidth = StageWidth();
	const int stageLeft = StageLeft();

	// 제목은 화면 폭에 들어가는 가장 큰 배율을 고름.
	// 120칸 화면에서 2배는 폭이 130이라 좌우가 잘렸음.
	const std::string titleText = "NO WAY DOWN";
	const int titleScale =
		BlockFont::MeasureWidth(titleText, 2) <= screenWidth ? 2 : 1;

	// 화면이 넉넉하면 큰 글자로, 좁으면 작은 글자로 떨어뜨림.
	// 이 판단을 안 하면 화면 크기를 바꿀 때마다 글자가 화면 밖으로 나감.
	const int consoleHeight = Engine::Get().GetConsoleHeight();
	const bool roomy = consoleHeight >= 40;

	const int titleY = roomy ? consoleHeight / 8 : 1;

	BlockFont::Draw(
		titleText,
		CenteredX(BlockFont::MeasureWidth(titleText, titleScale)),
		titleY,
		titleScale, 1,
		Color::ClothLight);

	const std::string subtitle = "SOMETHING IS ALREADY AWAKE";
	const int subtitleY = titleY + BlockFont::MeasureHeight(1) + 2;

	if (roomy && BlockFont::MeasureWidth(subtitle, 1) <= screenWidth)
	{
		BlockFont::Draw(
			subtitle,
			CenteredX(BlockFont::MeasureWidth(subtitle, 1)),
			subtitleY,
			1, 1,
			Color::ClothDark);
	}
	else
	{
		DrawCenteredText(
			"Something is already awake.", subtitleY, Color::ClothDark);
	}

	// 측면 무대. 화면 전체가 아니라 중앙 플랫폼 위에만 배치함.
	SceneArt::DrawPanelProp(stageLeft + 5, standY - 6);
	SceneArt::DrawValveProp(stageLeft + 21, standY - 4);

	// 문 옆의 불안정한 경고등.
	const bool lightOn = static_cast<int>(blinkTimer / 0.22f) % 5 != 4;
	if (lightOn || doorOpening)
	{
		SceneArt::FillRect(doorX - 3, standY - SceneArt::DoorHeight() + 3,
			1, 1, Color::Danger, 7);
	}

	SceneArt::DrawDoor(doorX, standY, doorOpening);

	// 문이 열리는 순간에만 안쪽의 밝은 틈이 보이게 함.
	if (doorOpening)
	{
		SceneArt::FillRect(
			doorX + 5, standY - SceneArt::DoorHeight() + 3,
			SceneArt::DoorWidth() - 7, SceneArt::DoorHeight() - 6,
			Color::WallTop, 7);
	}

	// 닫힌 문 아래에도 희미한 빛이 있어 시선이 문으로 모이게 함.
	SceneArt::FillRect(
		doorX + 2, standY,
		SceneArt::DoorWidth() - 4, 1,
		lightOn || doorOpening ? Color::WallTop : Color::DoorEdge,
		7);

	SceneArt::DrawStandingPerson(
		static_cast<int>(personX), standY);

	SceneArt::FillRect(
		stageLeft, floorLineY, stageWidth, 1,
		Color::ClothLight, 5);

	// 문 앞에 서면 문 위에 안내가 뜸.
	// 화면 한가운데 큰 버튼을 두는 대신, 걸어가서 직접 열게 함.
	const bool blinkVisible =
		static_cast<int>(blinkTimer / 0.6f) % 2 == 0;

	// 안내는 자리가 있으면 블록 글꼴, 좁으면 작은 글자.
	// 블록 글꼴은 한 줄에 7행을 써서 30행 화면에는 들어가지 않음.
	const int lineStep = roomy ? BlockFont::MeasureHeight(1) + 2 : 1;
	const int firstLineY = floorLineY + (roomy ? 6 : 1);

	if (IsNearDoor())
	{
		if (blinkVisible)
		{
			const std::string prompt =
				doorOpening ? "OPENING..." : "[F] OPEN DOOR";

			// 문 바로 위에 띄워서 시선이 문으로 모이게 함.
			const int promptX = doorX + SceneArt::DoorWidth() / 2;
			const int promptY = standY - SceneArt::DoorHeight()
				- (roomy ? 8 : 0);

			if (roomy)
			{
				BlockFont::Draw(
					prompt,
					promptX - BlockFont::MeasureWidth(prompt, 1) / 2,
					promptY,
					1, 1,
					doorOpening ? Color::Danger : Color::WallTop);
			}
			else
			{
				Renderer::Get().SubmitCells(
					prompt,
					Vector2(
						promptX - static_cast<int>(prompt.size()) / 2,
						promptY),
					doorOpening ? Color::Danger : Color::WallTop,
					10);
			}
		}
	}
	else if (roomy)
	{
		BlockFont::Draw(
			"WALK TO THE DOOR",
			CenteredX(BlockFont::MeasureWidth("WALK TO THE DOOR", 1)),
			firstLineY,
			1, 1,
			Color::ClothDark);
	}
	else
	{
		DrawCenteredText("Walk to the door.", firstLineY, Color::ClothDark);
	}

	if (roomy)
	{
		BlockFont::Draw(
			"ARROWS : MOVE    F : OPEN",
			CenteredX(BlockFont::MeasureWidth("ARROWS : MOVE    F : OPEN", 1)),
			firstLineY + lineStep,
			1, 1,
			Color::ClothDark);

		BlockFont::Draw(
			"Z : ATTACK    H : HEAL    ESC : QUIT",
			CenteredX(
				BlockFont::MeasureWidth("Z : ATTACK    H : HEAL    ESC : QUIT", 1)),
			firstLineY + lineStep * 2,
			1, 1,
			Color::ClothDark);
	}
	else
	{
		DrawCenteredText(
			"Arrows : Move    F : Open",
			firstLineY + lineStep, Color::ClothDark);

		DrawCenteredText(
			"Z : Attack    H : Heal    ESC : Quit",
			firstLineY + lineStep * 2, Color::ClothDark);
	}

	Level::Draw();
}
