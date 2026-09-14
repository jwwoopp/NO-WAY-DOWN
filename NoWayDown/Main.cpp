#include <iostream>
#include <Engine/Engine.h>
#include <Game/RunState.h>
#include <Level/FloorLevel.h>
#include <Level/TitleLevel.h>
#include "Level/TestLevel.h"
#include <memory>
#include "Audio/AudioSystem.h"
#include <Input/Input.h>
#include <cstring>

// Isolated font test: no zombies, combat, or game-state updates.
class HalfBlockPreviewLevel : public Craft::Level
{
public:
	void Tick(float) override
	{
		if (Craft::Input::Get().GetKeyDown(VK_ESCAPE))
			Craft::Engine::Get().Quit();
	}
	void Draw() override {}
};

int main(int argc, char* argv[])
{
	const bool preview = argc > 1 && std::strcmp(argv[1], "--half-block-test") == 0;
	// Process-local switch, set before the engine creates its screen buffer.
	SetEnvironmentVariableW(L"NWD_HALF_BLOCK_TEST", preview ? L"1" : nullptr);
	Craft::Engine engine;

	// 소리 파일을 미리 열어둠. 없어도 게임은 그대로 돌아감.
	AudioSystem::Get().Initialize();
	if (preview)
	{
		engine.AddNewLevel<HalfBlockPreviewLevel>();
		engine.Run();
		return 0;
	}
	std::shared_ptr<RunState> runState = std::make_shared<RunState>();
	engine.AddNewLevel<TitleLevel>(runState);
	// engine.AddNewLevel<TestLevel>(); // Keep the isolated rendering preview.
	engine.Run();

	AudioSystem::Get().Shutdown();
	return 0;
}
