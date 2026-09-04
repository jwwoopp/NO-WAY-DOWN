#pragma once

#include <Core/Core.h>
#include <memory>

namespace Craft
{
	class Level;
	class Input;
	class Renderer;

	class CRAFT_API Engine
	{
		struct Setting
		{
			float framerate = 120.0f;
			int width = 40;
			int height = 25;
		};

	public:
		Engine();
		virtual ~Engine();

		void Run();
		void Quit();

		template<typename T, typename = std::enable_if_t<std::is_base_of<Level, T>::value>>
		void AddNewLevel()
		{
			nextLevel = std::make_shared<T>();
		}

		// Get은 공용 Engine을 찾고,
		static Engine& Get();

		// Getter는 설정에서 화면 너비와 높이를 꺼냄.
		int GetWidth() const { return setting.width; }
		int GetHeight() const { return setting.height; }

	protected:

		void ProcessInput();
		void SavePreviousInputStates();

		void OnInitialized();
		void BeginPlay();
		void Tick(float deltaTime);
		void Draw();

		// Setting.txt 읽고 setting에 저장.
		void LoadEngineSetting();
		Setting setting;

		bool isQuit = false;
		// Engine::Get()이 현재 Engine 객체 찾는 데 사용할 주소.
		static Engine* instance;

		std::shared_ptr<Level> mainLevel;
		std::shared_ptr<Level> nextLevel;
		std::unique_ptr<Input> input;
		std::unique_ptr<Renderer> renderer;
	};
}