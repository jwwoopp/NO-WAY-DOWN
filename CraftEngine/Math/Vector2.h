#pragma once

#include <Core/Core.h>
#include <Windows.h>
// Core.h는 DLL 공개용 CRAFT_API를 사용하기 위함. 
// Windows.h는 콘솔 좌표 타입 COORD를 사용하기 위해 포함.

namespace Craft
{
	//Vector2는 x, y 좌표를 하나로 묶는 클래스.
	class CRAFT_API Vector2
	{
	public:
		//생성할 때 값을 생략하면 기본 좌표는 (0, 0).
		Vector2(int x = 0, int y = 0);
		~Vector2() = default;

		// Vector2를 Windows의 COORD 타입이 필요한 곳에 전달하면
		// 자동으로 변환해주는 함수.
		// 1. const 객체 사용.
		operator COORD() const;
		// 2. 일반 객체에서 사용.
		operator COORD();

		// 연산자 오버로딩.
		Vector2 operator+(const Vector2& other) const;
		Vector2 operator-(const Vector2& other) const;
		Vector2 operator*(const Vector2& other) const;
		Vector2 operator/(const Vector2& other) const;

		// 대입 연산자.
		Vector2& operator=(const Vector2& other);

		// 비교 연산자.
		// 비교만 할뿐 현재 좌표를 바꾸지 않으므로 끝에 const.
		bool operator==(const Vector2& other) const;
		bool operator!=(const Vector2& other) const;	
		
		static Vector2 Zero;
		static Vector2 One;
		static Vector2 Right;
		static Vector2 Up;

		int x = 0;
		int y = 0;
	};


}

