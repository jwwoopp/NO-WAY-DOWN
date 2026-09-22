#include "Engine.h"
#include <Input/Input.h>
#include <Level/Level.h>
#include <Renderer/Renderer.h>
#include <Core/ResourcePath.h>
#include <cassert>
#include <iostream>
#include <Windows.h>

namespace Craft
{
	// Engine.h에서는 instance가 있다고 선언만 함.
	// 실제 주소 저장공간을 만들고, nullptr 설정.
	Engine* Engine::instance = nullptr;
	Engine::Engine()
	{
		// Engine이 이미 존재하면 중복 생성 방지.
		// 현재 객체 주소를 instance에 저장함.
		assert(!instance && "instance should be null");
		instance = this;

		// 그 다음 Setting.txt 읽음.
		// 나중에 Renderer 생성자가 화면 크기 사용할 수 있으므로 이 순서 필요.
		LoadEngineSetting();

		// Renderer의 전체 구조를 Engine.cpp에게 알려줌.
		input = std::make_unique<Input>();

		// Engine 생성 시 실제 Renderer 객체 하나를 힙에 생성.
		// 힙에 만들어진 실제 Render 객체의 주소를 unique_ptr에 넣음.
		// Engine이 살아있는 동안 Renderer도 유지됨.
		// Engine이 사라지면 unique_ptr이 Renderer을 자동 삭제.
		
		// 설정 화면에서 읽은 화면 크기를 Renderer에 넘김.
		renderer = std::make_unique<Renderer>(Vector2(GetWidth(), GetHeight())
		);
	}
	Engine::~Engine()
	{
		// Engine 객체가 사라졌는데 instance가 이전 주소를 들고있으면
		// 잘못된 메모리를 가리킬 수 있어서 Engine 없음 상태로 되돌림.
		instance = nullptr;
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
			// 시간 계산.
			// 프레임 처리.

			ProcessInput();

			QueryPerformanceCounter(&counter);
			current = counter.QuadPart;

			// Snipping Tool 같은 전역 오버레이가 열린 동안에는
			// 콘솔 출력과 게임 시뮬레이션을 잠시 멈춤.
			// 120fps 설정은 그대로이고, 포커스를 되찾으면 즉시 재개함.
			if (input && !input->IsFocused())
			{
				previous = current;
				SavePreviousInputStates();
				Sleep(1);
				continue;
			}

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

	Engine& Engine::Get()
	{
		assert(instance && "instance should not be null");
		return *instance;
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

	void Engine::LoadEngineSetting()
	{
		const std::filesystem::path settingPath =
			ResolveResourcePath(L"Config/Setting.txt");
		FILE* file = nullptr;
		if (!settingPath.empty())
		{
			_wfopen_s(&file, settingPath.c_str(), L"rt");
		}

		if (!file)
		{
			std::cerr << "Could not open Config/Setting.txt beside the game files. "
				"Using default settings.\n";
			return;
		}
		// 파일 내용 담을 상자.
		const int bufferSize = 2048;
		// 상자 안을 전부 0으로 채움.
		char buffer[bufferSize] = {};

		size_t readSize
			// (buffer, sizeof(char), bufferSize, file)
			// 어디에 담을지, 한 칸 크기, 몇 칸 까지, 어느 파일에서.
			= fread(buffer, sizeof(char), bufferSize - 1, file);
		buffer[readSize] = '\0';
		const int extraCharacter = fgetc(file);
		if (ferror(file) || extraCharacter != EOF)
		{
			std::cerr << "Could not read Config/Setting.txt or the file is too large. "
				"Using default settings.\n";
			fclose(file);
			return;
		}

		char* context = nullptr;
		char* token = nullptr;
		// 줄 단위로 자르기.
		token = strtok_s(buffer, "\n", &context);
		// 첫 호출은 buffer, 그 다음부터는 nullptr
		// nullptr은 새로 시작하지 말고 하던 데에서 이어서.
		// 더 자를게 없으면 nullptr 반환.
		// 따라서 아래의 while 문에서 자동으로 끝남.

		while (token)
		{
			char key[15] = {};
			sscanf_s(token, "%s", key, 15);
			// 공백 전까지 읽어 key에 담음.
			// %s는 공백을 만나면 멈춤.

			if (strcmp(key, "framerate") == 0)
			// 두 글자 열이 같으면 0을 돌려줌.
			// 같으면 true가 아니라 0 반환.
			// 앞이면 음수, 뒤면 양수, 같으면 0.
			{
				sscanf_s(token, "framerate = %f", &setting.framerate);
				// 형식을 통째로 적어두고 값만 뽑아냄.
			}
			else if (strcmp(key, "width") == 0)
			{
				sscanf_s(token, "width = %d", &setting.width);
			}
			else if (strcmp(key, "height") == 0)
			{
				sscanf_s(token, "height = %d", &setting.height);
			}

			token = strtok_s(nullptr, "\n", &context);
		}
		
		fclose(file);
		file = nullptr;

		// 화면 크기는 설정 파일 값을 그대로 씀.
		// 터미널 창 크기를 그대로 따라가게 했더니, 창이 세로로 짧을 때
		// 배치가 전부 겹쳐서 오히려 더 망가졌음.
		// 대신 ScreenBuffer가 이 크기를 창 한가운데 놓고
		// 남는 가장자리를 배경색으로 채움.
	}
}
