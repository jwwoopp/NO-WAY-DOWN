#include "Input.h"
#include <cassert>
#include <Windows.h>

namespace Craft
{
	Input* Input::instance = nullptr;

	Input::Input()
	{
		assert(!instance && "instance should be null here");
		instance = this;
		consoleInput = GetStdHandle(STD_INPUT_HANDLE);
		if (GetConsoleMode(consoleInput, &originalInputMode))
		{
			DWORD mode = originalInputMode
				| ENABLE_MOUSE_INPUT
				| ENABLE_WINDOW_INPUT
				| ENABLE_EXTENDED_FLAGS;
			mode &= ~(ENABLE_QUICK_EDIT_MODE | ENABLE_VIRTUAL_TERMINAL_INPUT);
			restoreInputMode = SetConsoleMode(consoleInput, mode) != FALSE;
		}
	}

	Input::~Input()
	{
		if (restoreInputMode) SetConsoleMode(consoleInput, originalInputMode);
		instance = nullptr;
	}

	bool Input::GetMouseClick(COORD& position) const
	{
		position = mouseClickPosition;
		return mouseClicked;
	}

	bool Input::GetKeyDown(int keyCode) const
	{
		return !keyStates[keyCode].wasKeyDown && keyStates[keyCode].isKeyDown;
	}

	bool Input::GetKeyUp(int keyCode) const
	{
		return keyStates[keyCode].wasKeyDown && !keyStates[keyCode].isKeyDown;
	}

	bool Input::GetKey(int keyCode) const
	{
		return keyStates[keyCode].isKeyDown;
	}

	Input& Input::Get()
	{
		assert(instance && "instance should not be null here");
		return *instance;
	}

	void Input::ProcessInput()
	{
		// Drain only available records; never block the game loop.
		DWORD pending = 0;
		if (restoreInputMode && GetNumberOfConsoleInputEvents(consoleInput, &pending))
		{
			INPUT_RECORD records[64];
			DWORD read = 0;
			const DWORD count = pending < 64 ? pending : 64;
			if (count && ReadConsoleInputW(consoleInput, records, count, &read))
			{
				for (DWORD i = 0; i < read; ++i)
				{
					if (records[i].EventType == FOCUS_EVENT)
					{
						consoleFocused = records[i].Event.FocusEvent.bSetFocus != FALSE;
						if (!consoleFocused)
						{
							mouseHeld = false;
							mouseClicked = false;
						}
					}
					if (records[i].EventType != MOUSE_EVENT) continue;
					const auto& mouse = records[i].Event.MouseEvent;
					const bool down = (mouse.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) != 0;
					if (down && !mouseHeld && (mouse.dwEventFlags == 0 || mouse.dwEventFlags == DOUBLE_CLICK))
					{
						mouseClickPosition = mouse.dwMousePosition;
						mouseClicked = true;
					}
					mouseHeld = down;
				}
			}
		}
		for (int ix = 0; ix < keyCount; ++ix)
		{
			keyStates[ix].isKeyDown = ((GetAsyncKeyState(ix) & 0x8000) != 0);
		}

		// 캡처/전환 중에는 Win·Shift 같은 전역 단축키가 게임에 남지 않게 함.
		// 포커스를 되찾은 뒤 새로 눌린 키만 정상적으로 입력으로 취급함.
		if (!consoleFocused)
		{
			for (KeyState& state : keyStates)
			{
				state.isKeyDown = false;
				state.wasKeyDown = false;
			}
		}
	}

	void Input::SavePreviousStates()
	{
		// Keep clicks latched across input polls until one simulation tick consumes them.
		mouseClicked = false;
		for (KeyState& state : keyStates)
		{
			state.wasKeyDown = state.isKeyDown;
		}
	}

}
