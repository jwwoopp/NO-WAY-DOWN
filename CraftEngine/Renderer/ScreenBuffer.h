#pragma once

#include <Math/Vector2.h>
#include <Windows.h>

namespace Craft
{
	class ScreenBuffer
	{
	public:
		ScreenBuffer(const Vector2& screenSize);
		~ScreenBuffer();

		// const 붙이는 이유 : 콘솔 화면 내용만 지우고
		// 멤버 변수는 그대로임(buffer, size).
		void Clear() const;
		// 글자 하나 + 색 하나.
		// 화면이 40x25면 총 1000개.
		void Draw(const CHAR_INFO* const charInfo) const;

		// 콘솔 화면 한 장.
		inline HANDLE GetBuffer() const { return buffer; }

	private:
		HANDLE buffer = nullptr;

		Vector2 size;
	};
}