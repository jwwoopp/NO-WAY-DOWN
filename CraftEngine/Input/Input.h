#pragma once

#include <Core/Core.h>
#include <Windows.h>

namespace Craft
{
	class CRAFT_API Input
	{
		friend class Engine;

		struct KeyState
		{
			bool isKeyDown = false;
			bool wasKeyDown = false;
		};

	public:
		Input();
		~Input();
		bool GetMouseClick(COORD& position) const;

		bool GetKeyDown(int keyCode) const;
		bool GetKeyUp(int keyCode) const;
		bool GetKey(int keyCode) const;
		// Windows 캡처 오버레이처럼 콘솔이 잠시 포커스를 잃었는지 확인함.
		bool IsFocused() const { return consoleFocused; }

		static Input& Get();

	private:
		void ProcessInput();
		void SavePreviousStates();

	private:
		const int keyCount = 256;
		KeyState keyStates[256] = { };

		static Input* instance;
		HANDLE consoleInput = INVALID_HANDLE_VALUE;
		DWORD originalInputMode = 0;
		bool restoreInputMode = false;
		bool consoleFocused = true;
		bool mouseHeld = false;
		bool mouseClicked = false;
		COORD mouseClickPosition = {};
	};
}
