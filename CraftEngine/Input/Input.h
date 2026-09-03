#pragma once

namespace Craft
{
	class Input
	{
		friend class Engine;

		struct KeyState
		{
			bool isKeyDown = false;
			bool wasKeyDown = false;
		};

	public:
		Input();
		~Input() = default;

		bool GetKeyDown(int keyCode) const;
		bool GetKeyUp(int keyCode) const;
		bool GetKey(int keyCode) const;

		static Input& Get();

	private:
		void ProcessInput();
		void SavePreviousStates();

	private:
		const int keyCount = 256;
		KeyState keyStates[256] = { };

		static Input* instance;
	};
}