#pragma once

#include <Core/Core.h>
#include <Actor/Actor.h>
#include <Core/CraftObject.h>
#include <memory>
#include <vector>

namespace Craft
{
	class Engine;

	class CRAFT_API Level :
		public CraftObject,
		public std::enable_shared_from_this<Level>
	{
		// 커스텀 타입 설정.
		TYPE_DECLARATIONS(Level, CraftObject)

		friend class Engine;

		public:
			Level();
			virtual ~Level();

			virtual void OnInitialized();
			virtual void BeginPlay();
			virtual void Tick(float deltaTime);
			virtual void Draw();

			bool HasInitialized() const { return hasInitialized; }

		protected:
			void ProcessAddAndDestroyActors();

			bool hasInitialized = false;

			std::vector<std::shared_ptr<Actor>> actorList;
			std::vector<std::shared_ptr<Actor>> addRequestedActorList;

		public:
			template<typename T, typename... Args,
				typename = std::enable_if_t<std::is_base_of<Actor, T>::value>>
				std::shared_ptr<T> SpawnActor(Args&&... args)
			{
				std::shared_ptr<T> newActor =
					std::make_shared<T>(std::forward<Args>(args)...);

				addRequestedActorList.emplace_back(newActor);
				newActor->SetOwner(weak_from_this());

				return newActor;
			}
	};
}