#include "Vector2.h"
#include <cassert>
// Vector2.h가 이미 Core.h와 Windows.h를 포함함. 
// 따라서 cpp에서 다시 포함할 필요는 없음.

namespace Craft
{
	// Vector2는 클래스 이름이라 반드시 대문자.
	// 두번째 Vector2도 클래스에 속한 멤버라 대문자.
	Vector2 Vector2::Zero(0, 0);
	Vector2 Vector2::One(1, 1);
	Vector2 Vector2::Right(1, 0);
	Vector2 Vector2::Up(0, -1);

	Vector2::Vector2(int x, int y)
		: x(x), y(y)
		// 괄호 안의 소문자 x, y - 외부에서 전달받은 매개변수.
		// 콜론 뒤 x, y - 멤버 변수 x, y에 매개변수 x를 삽입.
	{
		
	}

	Vector2::operator COORD() const
	{
		COORD coord = {};
		coord.X = static_cast<short>(x);
		coord.Y = static_cast<short>(y);

		return coord;
	}

	Vector2::operator COORD()
	{
		COORD coord = {};
		coord.X = static_cast<short>(x);
		coord.Y = static_cast<short>(y);

		return coord;
	}

	Vector2 Vector2::operator+(const Vector2& other) const
	{
		return Vector2(x + other.x, y + other.y);
	}

	Vector2 Vector2::operator-(const Vector2& other) const
	{
		return Vector2(x - other.x, y - other.y);
	}

	Vector2 Vector2::operator*(const Vector2& other) const
	{
		return Vector2(x * other.x, y * other.y);
	}

	Vector2 Vector2::operator/(const Vector2& other) const
	{
		// other의 x와 y가 0이면 디버그 실행을 멈춰 0 나눗셈을 알려줌.
		assert(other.x != 0 && other.y != 0);
		return Vector2(x / other.x, y / other.y);
	}

	Vector2& Vector2::operator=(const Vector2& other)
	{
		// other의 좌표를 현재 객체에 복사.
		x = other.x;
		y = other.y;

		// this는 현재 객체의 주소. *this는 현재 객체 자신.
		// 자기 자신을 참조로 반환하여 연속 대입이 가능해짐.
		return *this;
	}

	// 두 좌표가 같은 위치인지 쉽게 확인하기 위해 만듦.
	// if (player.x == exit.x && player.y == exit.y)
	// 비교연산자로 간단하게.

	// x와 y가 모두 같으면 true 반환.
	// 출구 도착 판정 사용.

	bool Vector2::operator==(const Vector2& other) const
	{
		return (x == other.x) && (y == other.y);
	}

	// x나 y 중 하나라도 다르면 true 반환.
	// 목표 도착 판정 사용.
	bool Vector2::operator!=(const Vector2& other) const
	{
		return !(*this == other);
	}

	// playerPosition, exitPosition의 실제 변수 이름을 미리 알 수 없음.
	// 그래서 왼쪽은 this, 오른쪽은 other이라는 공통 이름 처리.
	
}