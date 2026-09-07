#pragma once

#include <Core/Core.h>
#include <memory>
#include <utility>

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

		// RunState 같은 생성자도 함께 전달하기 위해 변경.
		// AddNewLevel을 호출하면 runState가 args안에 들어감.
		// std::forward<Args>(args)...가 그 값을 make_shared<T>에 넘김.
		// make_shared가 실행되므로 결과적으로 FloorLevel 생성자가 값을 받음.
		template<typename T, typename... Args>
		void AddNewLevel(Args&&... args)
		{
			nextLevel = std::make_shared<T>(std::forward<Args>(args)...);
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