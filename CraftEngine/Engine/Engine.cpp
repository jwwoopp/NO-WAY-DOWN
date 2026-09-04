#include "Engine.h"
#include <Input/Input.h>
#include <Level/Level.h>
#include <Renderer/Renderer.h>
#include <cassert>
#include <iostream>
#include <Windows.h>

namespace Craft
{
	Engine::Engine()
	{
		// Renderer의 전체 구조를 Engine.cpp에게 알려줌.
		input = std::make_unique<Input>();

		// Engine 생성 시 실제 Renderer 객체 하나를 힙에 생성.
		// 힙에 만들어진 실제 Render 객체의 주소를 unique_ptr에 넣음.
		// Engine이 살아있는 동안 Renderer도 유지됨.
		// Engine이 사라지면 unique_ptr이 Renderer을 자동 삭제.
		renderer = std::make_unique<Renderer>();
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

			ProcessInput();

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

				SavePreviousInputStates();

				previous = current;
			}
		}
	}

	void Engine::Quit()
	{
		isQuit = true;
	}

	void Engine::ProcessInput()
	{
		assert(input && "input should not be null here");
		input->ProcessInput();
	}

	void Engine::SavePreviousInputStates()
	{
		assert(input && "input should not be null here");
		input->SavePreviousStates();
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

		if (!renderer)
		{
			return;
		}

		renderer->Draw();
	}
}