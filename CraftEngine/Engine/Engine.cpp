#include "Engine.h"
#include <iostream>
#include <Windows.h>
#include <Level/Level.h>

namespace Craft
{
	Engine::Engine()
	{
	}
	Engine::~Engine()
	{
	}
	void Engine::Run()
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);

		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);

		int64_t current = counter.QuadPart;
		int64_t previous = current;

		float oneFrameTime = 1.0f / setting.framerate;

		while (!isQuit)
		{
			// ESC 처리.
			// 시간 계산.
			// 프레임 처리.

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
				OnInitialized();
				BeginPlay();
				Tick(deltaTime);
				Draw();

				if (nextLevel)
				{
					mainLevel = nextLevel;
					nextLevel.reset();
				}

				if (mainLevel)
				{
					mainLevel->ProcessAddAndDestroyActors();
				}

				previous = current;
			}
		}
	}

	void Engine::Quit()
	{
		isQuit = true;
	}

	void Engine::OnInitialized()
	{
		if (!mainLevel || mainLevel->HasInitialized())
		{
			return;
		}
		mainLevel->OnInitialized();
	}

	void Engine::BeginPlay()
	{
		if (!mainLevel)
		{
			return;
		}
		mainLevel->BeginPlay();
	}

	void Engine::Tick(float deltaTime)
	{
		if (!mainLevel)
		{
			return;
		}
		mainLevel->Tick(deltaTime);
	}

	void Engine::Draw()
	{
		if (!mainLevel)
		{
			return;
		}

		mainLevel->Draw();
	}
}