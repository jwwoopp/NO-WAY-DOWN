#include "Level.h"

namespace Craft
{
	Level::Level()
	{

	}

	Level::~Level()
	{
	}

	void Level::OnInitialized()
	{
		hasInitialized = true;
	}

	void Level::BeginPlay()
	{
		for (const std::shared_ptr<Actor>& actor : actorList)
		{
			if (actor->HasBeganPlay())
			{
				continue;
			}
			actor->BeginPlay();
		}
	}

	void Level::Tick(float deltaTime)
	{
		for (const std::shared_ptr<Actor>& actor : actorList)
		{
			if (!actor->IsActive())
			{
				continue;
			}
			
			actor->Tick(deltaTime);
		}
	}
	
	void Level::Draw()
	{
		for (const std::shared_ptr<Actor>& actor : actorList)
		{
			if (!actor->IsActive())
			{
				continue;
			}
			actor->Draw();
		}
	}
	void Level::ProcessAddAndDestroyActors()
	{
		for (auto iterator = actorList.begin(); iterator != actorList.end();)
		{
			std::shared_ptr<Actor> actor = *iterator;

			if (actor->HasExpired())
			{
				iterator = actorList.erase(iterator);
				continue;
			}

			++iterator;
		}

		if (addRequestedActorList.empty())
		{
			return;
		}

		for (const std::shared_ptr<Actor>& actor : addRequestedActorList)
		{
			actorList.emplace_back(actor);
		}

		addRequestedActorList.clear();
	}
}