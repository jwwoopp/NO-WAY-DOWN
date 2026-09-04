#include <iostream>
#include <Engine/Engine.h>
#include <Level/FloorLevel.h>

int main()
{
	Craft::Engine engine;
	engine.AddNewLevel<FloorLevel>();
	engine.Run();
	return 0;
}