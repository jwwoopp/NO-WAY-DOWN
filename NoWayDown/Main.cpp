#include <iostream>
#include <Engine/Engine.h>
#include <Game/RunState.h>
#include <Level/FloorLevel.h>
#include <memory>

int main()
{
	Craft::Engine engine;
	std::shared_ptr<RunState> runState = std::make_shared<RunState>();
	engine.AddNewLevel<FloorLevel>(runState);
	engine.Run();
	return 0;
}