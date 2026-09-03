#include "TestActor.h"
#include <Input/Input.h>
#include <iostream>

using namespace Craft;

void TestActor::Tick(float deltaTime)
{
	Actor::Tick(deltaTime);

	if (Input::Get().GetKeyDown('A'))
	{
		std::cout << "A Key is down\n";
	}

	if (Input::Get().GetKey('A'))
	{
		std::cout << "A Key is holding down\n";
	}

	if (Input::Get().GetKeyUp('A'))
	{
		std::cout << "A Key is up\n";
	}

}
