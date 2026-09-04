#pragma once
#include <Core/Core.h>
// Actor가 화면 문자 string, 위치 Vectotr2, 색상 Color를 저장하기 위해 필요한 헤더.
#include <Math/Vector2.h>
#include <Math/Color.h>
#include <memory>
#include <string>


namespace Craft
{

	class Level;
	
	class CRAFT_API Actor
	{
	public:
		Actor(
			const std::string& image = "",
			const Vector2& position = Vector2::Zero,
			Color color = Color::White
		);
		virtual ~Actor();

		virtual void BeginPlay();
		virtual void Tick(float deltaTime);
		virtual void Draw();

		void Destroy();

		void QuitGame();

		inline bool HasBeganPlay() const
		{
			return hasBeganPlay;
		}

		inline bool IsActive() const
		{
			return isActive && !hasExpired;
		}

		inline bool HasExpired() const
		{
			return hasExpired;
		}

		std::shared_ptr<Level> GetOwner() const { return owner.lock(); }
		void SetOwner(std::weak_ptr<Level> newOwner) { owner = newOwner; }

		// Actor의 현재 위치 반환.
		Vector2 GetPosition() const { return position; }
		// 새로운 위치로 이동시키는 함수.
		void SetPosition(const Vector2& newPosition);

	protected:

		bool hasBeganPlay = false;
		bool isActive = true;
		bool hasExpired = false;

		std::weak_ptr<Level> owner;

		// 각 Actor가 화면 문자, 색상, 문자 길이, 그리기 순서, 위치 저장.
		std::string image;
		Color color = Color::White;
		int width = 0;
		int sortingOrder = 0;
		Vector2 position;

	};

}
