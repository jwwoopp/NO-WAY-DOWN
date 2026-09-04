#pragma once

#include <Core/Core.h>
#include <memory>

namespace Craft
{
	class CRAFT_API CraftObject
	{
	public:
		virtual ~CraftObject() = default;
		// GetType은 객체 자신의 타입 번호 반환.
		// =0이기 때문에 자식 클래스가 반드시 구현해야 함.
		virtual size_t GetType() const = 0;

		// Is는 전달받은 타입 번호와 일치하는 지 검사.
		// 최상위 CraftObject까지 왔다면 일치 타입 없으므로 false 반환.
		virtual bool Is(size_t id) const
		{
			return false;
		}

		// 사용할 때 숫자 ID를 직접 전달하지 않고, 타입 이름으로 질문하게 해줌.
		template<typename T> bool IsTypeOf() const
		{
		// 내부에서는 Wall의 타입 번호를 얻어 Is 함수에 전달.
			return Is(T::TypeId());
		}
	};

	// U는 현재 포인터 타입.
	// T는 바꾸려는 타입.

	template<typename T, typename U>
	std::shared_ptr<T> Cast(const std::shared_ptr<U>& object)
	{
		// actor가 비어 있으면 nullptr 반환.
		if (!object)
		{
			return nullptr;
		}

		// 비어있지 않으면 실제 객체가 Wall인지 판단.
		if (object->Is(T::TypeId()))
		{
			// Wall이 맞으면 Wall 포인터로 반환.
			return std::static_pointer_cast<T>(object);
		}
		// 아니면 nullptr 반환.
		return nullptr;
	}
}


// 타입 시스템을 사용하는 클래스에 배치할 매크로.
#define TYPE_DECLARATIONS(Type, ParentType)							\
	using super = ParentType;										\
protected:															\
	/* 전역 지역 변수의 주소를 활용해 유니크한 id를 반환하는 함수 */			\
	static size_t TypeIdClass()										\
	{																\
		static int runTimeTypeId = 0;								\
		return reinterpret_cast<size_t>(&runTimeTypeId);			\
	}																\
public:																\
	static size_t TypeId()											\
	{																\
		return Type::TypeIdClass();									\
	}																\
	virtual size_t GetType() const override							\
	{																\
		return Type::TypeIdClass();									\
	}																\
	virtual bool Is(size_t id) const override						\
	{																\
		/* 현재 계층에서 일단 비교하고, 타입이 다르면 부모 계층까지 검색 */ \
		return (id == TypeIdClass()) ? true : ParentType::Is(id);	\
	}