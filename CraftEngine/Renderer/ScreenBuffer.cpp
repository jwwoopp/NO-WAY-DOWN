#include "ScreenBuffer.h"
#include <cassert>
#include <iostream>

namespace Craft
{
	ScreenBuffer::ScreenBuffer(const Vector2& screenSize)
		// 초기화 리스트.
		// size를 screenSize로 만들면서 시작.
		: size(screenSize)
	{
		buffer = CreateConsoleScreenBuffer(
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr,
			CONSOLE_TEXTMODE_BUFFER,
			nullptr
		);

		assert(buffer != INVALID_HANDLE_VALUE);

		// 창 크기.
		SMALL_RECT rect = {};
		rect.Top = 0;
		rect.Left = 0;
		// size.x - 1
		// -1이 붙는 이유는 좌표가 0에서 시작하기 때문.
		rect.Right = static_cast<short>(size.x - 1);
		rect.Bottom = static_cast<short>(size.y - 1);
		// operator COORD().
		BOOL result = SetConsoleWindowInfo(buffer, TRUE, &rect);

		assert(result == TRUE);

		result = SetConsoleScreenBufferSize(buffer, size);
		assert(result == TRUE);
		
		// 커서 설정은 화면 한 장마다 따로.
		// Renderer에서 껐지만 커서를 또 끔.
		CONSOLE_CURSOR_INFO info;
		result = GetConsoleCursorInfo(buffer, &info);
		assert(result == TRUE);

		info.bVisible = FALSE;
		result = SetConsoleCursorInfo(buffer, &info);
		assert(result == TRUE);
	}

	ScreenBuffer::~ScreenBuffer()
	{
		if (buffer)
		{
			// Windows에서 빌려온 건 돌려줘야 함.
			// 안 그러면 프로그램 끝날 때까지 자원 물려있음.
			// = fclose
			CloseHandle(buffer);
		}
	}

	void ScreenBuffer::Clear() const
	{
		DWORD writtenCount = 0;

		// 화면 지우기
		// (어느 화면을, 무엇으로, 몇 칸, 어디서부터, 실제로 몇 칸 채우는지 받을 곳).
		BOOL result = FillConsoleOutputCharacterA(
			buffer,
			' ',
			size.x * size.y,
			Vector2::Zero,
			&writtenCount
		);

		assert(result == TRUE);
	}

	// CHAR_INFO* 앞에 const.
	// charInfo 앞에 const.
	// const 두 개가 붙어서 가리키는 내용과 포인터 자체.
	// 변경 불가, 다른데 못가리키게 함.
	void ScreenBuffer::Draw(const CHAR_INFO* const charInfo) const
	{
		SMALL_RECT rect = {
			0,
			0,
			static_cast<short>(size.x),
			static_cast<short>(size.y)
		};

		// 배열 1000개를 한 번의 호출로 화면에 씀.
		BOOL result = WriteConsoleOutputA(
			buffer,
			charInfo,
			size,
			Vector2::Zero,
			&rect
		);

		assert(result == TRUE);
	}

}