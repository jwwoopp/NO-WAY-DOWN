#include "EndingLevel.h"
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
	int CenteredX(int contentWidth)
	{
		return (Engine::Get().GetWidth() - contentWidth) / 2;
	}

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

EndingLevel::EndingLevel(const std::shared_ptr<RunState>& newRunState)
	: runState(newRunState)
{
	cleared = runState->isCleared;
	reachedFloor = runState->currentFloor;
}

void EndingLevel::StartNewRun()
{
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

void EndingLevel::Tick(float deltaTime)
{
	blinkTimer += deltaTime;

	if (Input::Get().GetKeyDown(VK_ESCAPE))
	{
		Engine::Get().Quit();
		return;
	}

	if (Input::Get().GetKeyDown('R'))
	{
		StartNewRun();
		return;
	}

	AudioSystem::Get().Update(deltaTime);
	Level::Tick(deltaTime);
}

void EndingLevel::Draw()
{
	// 좌표를 상수로 박아두면 화면 크기를 바꿀 때마다 전부 어긋남.
	// 실제로 160x80 기준으로 박아둔 값들이 120x30에서 화면 밖으로 나갔음.
	// 그래서 지금은 화면 크기에서 계산함.
	//
	// SceneArt와 BlockFont는 SubmitCells를 쓰므로 세로 단위가 "셀"임.
	// GetHeight()는 논리 높이(셀의 두 배)라 여기서 쓰면 안 됨.
	const int screenWidth = Engine::Get().GetWidth();
	const int consoleHeight = Engine::Get().GetConsoleHeight();

	// 화면이 넉넉하면 안내를 블록 글꼴로 쓰므로 바닥선을 더 위로 올림.
	const bool roomy = consoleHeight >= 40;

	const int floorLineY = roomy ? consoleHeight - 20 : consoleHeight - 4;
	const int standY = floorLineY - 1;

	const int stageWidth = screenWidth / 2;
	const int stageLeft = (screenWidth - stageWidth) / 2;

	// 성공과 실패로 문구와 색을 나눔.
	const Color headlineColor = cleared ? Color::Stairs : Color::Danger;
	const std::string headline = cleared ? "YOU ESCAPED" : "YOU DIED";

	// 화면 폭에 들어가는 가장 큰 배율을 고름.
	const int headlineScale =
		BlockFont::MeasureWidth(headline, 2) <= screenWidth ? 2 : 1;

	const int headlineY = roomy ? consoleHeight / 8 : 1;

	BlockFont::Draw(
		headline,
		CenteredX(BlockFont::MeasureWidth(headline, headlineScale)),
		headlineY,
		headlineScale, 1,
		headlineColor);

	const int textY = headlineY + BlockFont::MeasureHeight(1) + 2;

	if (cleared)
	{
		DrawCenteredText(
			"You walked out of the building alive.",
			textY, Color::ClothDark);
	}
	else
	{
		DrawCenteredText(
			"Floor " + std::to_string(reachedFloor)
			+ " is where it caught you.",
			textY, Color::ClothDark);
	}

	DrawCenteredText(
		"Reached floor: " + std::to_string(reachedFloor) + " / 12",
		textY + 1, Color::Skin);

	// 측면 무대. 탈출은 열린 문 밖에 서 있고, 사망은 닫힌 문 앞에 쓰러져 있음.
	SceneArt::DrawPanelProp(stageLeft + stageWidth - 18, standY - 6);
	SceneArt::DrawValveProp(stageLeft + stageWidth - 6, standY - 4);

	SceneArt::DrawDoor(stageLeft, standY, cleared);

	if (cleared)
	{
		SceneArt::DrawStandingPerson(stageLeft + stageWidth / 2, standY);
	}
	else
	{
		SceneArt::DrawFallenPerson(stageLeft + stageWidth / 2, standY);
	}

	SceneArt::FillRect(
		stageLeft, floorLineY, stageWidth, 1, Color::ClothLight, 5);

	// 바닥선 아래에 세 줄만 남으므로 안내는 작은 글자로 둠.
	const bool blinkVisible =
		static_cast<int>(blinkTimer / 0.6f) % 2 == 0;

	const int lineStep = roomy ? BlockFont::MeasureHeight(1) + 2 : 1;
	const int firstLineY = floorLineY + (roomy ? 5 : 2);

	if (blinkVisible)
	{
		if (roomy)
		{
			BlockFont::Draw(
				"R : RETRY",
				CenteredX(BlockFont::MeasureWidth("R : RETRY", 1)),
				firstLineY,
				1, 1,
				Color::WallTop);
		}
		else
		{
			DrawCenteredText("R : Retry", firstLineY, Color::WallTop);
		}
	}

	DrawCenteredText(
		"ESC : Quit", firstLineY + lineStep, Color::ClothDark);

	Level::Draw();
}
