#include <iostream>
#include <Engine/Engine.h>
#include <Level/TestLevel.h>

int main()
{
	Craft::Engine engine;
	engine.AddNewLevel<TestLevel>();
	engine.Run();
	return 0;
}