#include "Actor.h"

namespace Craft
{
	Actor::Actor()
	{
	}

	Actor::Actor()
	{
	}

	void Actor::BeginPlay()
	{
		hasBeganPlay = true;
	}

	void Actor::Tick(float deltaTime)
	{
	}

	void Actor::Draw()
	{
	}
	
	void Actor::Destory()
	{
		hasExpired = true;
	}


}