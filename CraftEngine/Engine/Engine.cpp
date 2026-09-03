#include "Engine.h"
#include <iostream>
#include <Windows.h>

namespace Craft
{
	void Engine::Run()
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);

		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);

		int64_t current = counter.QuadPart;
		int64_t previous = current;
		
		float oneFrameTime = 1.0f / setting.framerate;
		
		while (true)
		{
			if (isQuit)
			{
				break;
			}
		}
		while (!isQuit)
		{
			if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0)
			{
				Quit();
				continue;
			}

			QueryPerformanceCounter(&counter);
			current = counter.QuadPart;

			float deltaTime =
				static_cast<float>(current - previous) /
				static_cast<float>(frequency.QuadPart);

			if (deltaTime >= oneFrameTime)
			{
				std::cout << "DeltaTime: " << deltaTime << " | FPS: " << 1.0f / deltaTime << '\n';

				previous = current;
			}
		}

	}

	void Engine::Quit()
	{
	isQuit = true;
	}
}