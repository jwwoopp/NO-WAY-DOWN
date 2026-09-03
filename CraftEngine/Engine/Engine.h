#pragma once

namespace Craft
{
	class Engine
	{
		struct Setting
		{
			float framerate = 120.0f;
		};


	public:
		void Run();
		void Quit();

	protected:
		Setting setting;
		bool isQuit = false;
	};
}