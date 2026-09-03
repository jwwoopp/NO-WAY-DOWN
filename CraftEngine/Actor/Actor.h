#pragma once
#include <memory>

namespace Craft
{

	class Level;
	
	class Actor
	{
	public:
		Actor();
		virtual ~Actor();

		virtual void BeginPlay();
		virtual void Tick(float deltaTime);
		virtual void Draw();

		void Destroy();

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

	protected:

		bool hasBeganPlay = false;
		bool isActive = true;
		bool hasExpired = false;

		std::weak_ptr<Level> owner;

	};

}
