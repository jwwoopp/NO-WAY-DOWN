#include "GameHUD.h"
#include "BlockFont.h"
#include <Game/RunState.h>
#include <Renderer/Renderer.h>
#include <string>
#include <PathFinding/AStar.h>
#include <cstdio>
#include <Engine/Engine.h>

using namespace Craft;

void GameHUD::Draw(const RunState& runState)
{
	// 콘솔 기본 글자는 한 칸짜리라 게임 화면 위에서 거의 안 읽힘.
	// 그래서 상태는 블록 글꼴로 크게 그림.
	//
	// 조작법은 타이틀 화면에서 이미 크게 안내하므로 여기서는 빼고
	// 지금 당장 알아야 하는 값만 남김.
	const int lineHeight = BlockFont::MeasureHeight(1) + 2;
	const int gap = 8;

	const std::string floorText =
		"FLOOR " + std::to_string(runState.currentFloor);
	const std::string healthText =
		"HP " + std::to_string(runState.health);
	const std::string medicineText =
		"MED " + std::to_string(runState.medicineCount);
	const std::string ammoText =
		"AMMO " + std::to_string(runState.ammoCount);

	int cursorX = 2;
	const int firstLineY = 1;
	const int secondLineY = firstLineY + lineHeight;

	BlockFont::Draw(
		floorText, cursorX, firstLineY, 1, 1, Color::ClothLight);

	cursorX += BlockFont::MeasureWidth(floorText, 1) + gap;

	// 체력이 얼마 안 남으면 붉게 바뀌어서 숫자를 읽기 전에 먼저 눈에 들어옴.
	BlockFont::Draw(
		healthText, cursorX, firstLineY, 1, 1,
		runState.health <= 30 ? Color::Danger : Color::ClothLight);

	cursorX = 2;

	BlockFont::Draw(
		medicineText, cursorX, secondLineY, 1, 1, Color::Stairs);

	cursorX += BlockFont::MeasureWidth(medicineText, 1) + gap;

	// 탄약이 없으면 어둡게 해서 총이 안 나간다는 걸 미리 알림.
	BlockFont::Draw(
		ammoText, cursorX, secondLineY, 1, 1,
		runState.ammoCount > 0 ? Color::Skin : Color::ClothDark);

	// 카메라 회전은 플레이 중에만 쓰는 보조 조작이라 작게 표시함.
	Renderer::Get().SubmitCells(
		"Q/E ROTATE  M MAP",
		Vector2(Engine::Get().GetWidth() - 18, firstLineY),
		Color::ClothDark,
		11);

	if (runState.debugMode)
	{
		// 디버그 수치는 개발용이라 작은 글자 그대로 둠.
		const AStarStats& stats = AStar::GetStats();

		double average = 0.0;

		if (stats.searchCount > 0)
		{
			average = stats.totalMicroseconds / stats.searchCount;
		}

		char text[64] = {};
		const int debugY = secondLineY + lineHeight + 1;

		sprintf_s(text, "A* calls: %d", stats.searchCount);
		Renderer::Get().SubmitCells(text, Vector2(2, debugY), Color::Skin, 11);

		sprintf_s(text, "A* avg: %.1f us", average);
		Renderer::Get().SubmitCells(text, Vector2(2, debugY + 1), Color::Skin, 11);
	}

	if (runState.isGameOver || runState.isCleared)
	{
		// 결과 화면으로 넘어가기 전 1.2초 동안만 보이는 문구.
		const std::string message =
			runState.isCleared ? "ESCAPED" : "YOU DIED";

		BlockFont::Draw(
			message,
			(Engine::Get().GetWidth()
				- BlockFont::MeasureWidth(message, 2)) / 2,
			Engine::Get().GetConsoleHeight() / 2 - BlockFont::MeasureHeight(1),
			2, 1,
			runState.isCleared ? Color::Stairs : Color::Danger,
			12);
	}
}
