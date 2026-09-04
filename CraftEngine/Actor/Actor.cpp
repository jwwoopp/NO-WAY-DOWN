#include "Actor.h"
#include <Renderer/Renderer.h>

namespace Craft
{
	// 전달받은 값을 Actor 자신의 멤버에 저장.
	// width는 문자열 길이를 int로 변환해 저장.
	// 여러 글자의 화면 범위 계산할 때 사용.
	Actor::Actor(
	const std::string& image,
	const Vector2& position,
	Color color)
		: image(image),
		  color(color),
		  width(static_cast<int>(image.length())),
		  position(position)
	{
		
	}

	Actor::~Actor()
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
		// Actor는 직접 콘솔에 출력하지 않고
		// 자신의 문자, 위치, 색상, 순서를 Renderer에 전달.
		// Renderer는 모든 요청을 모았다가 Draw단계에서 한번에 처리.
		Renderer::Get().Submit(image, position, color, sortingOrder);
	}

	void Actor::SetPosition(const Vector2& newPosition)
	{
		// 외부에서 Actor의 위치를 변경할 때 사용하는 함수.
		// 플레이어가 방향키 누르면
		// 현재 위치 + 방향 Vector2 더해서 저장.
		position = newPosition;
	}

	void Actor::Destroy()
	{
		hasExpired = true;
	}


}