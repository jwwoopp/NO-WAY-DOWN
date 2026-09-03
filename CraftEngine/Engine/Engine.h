#pragma once

#include <memory>

namespace Craft
{
	class Level;
	class Input;

	class Engine
	{
		struct Setting
		{
			float framerate = 120.0f;
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

	protected:

		void ProcessInput();
		void SavePreviousInputStates();

		void OnInitialized();
		void BeginPlay();
		void Tick(float deltaTime);
		void Draw();

		Setting setting;
		bool isQuit = false;

		std::shared_ptr<Level> mainLevel;
		std::shared_ptr<Level> nextLevel;
		std::unique_ptr<Input> input;
	};
}