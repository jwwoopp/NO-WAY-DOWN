#pragma once

#include <Core/Core.h>
#include <Math/Vector2.h>
#include <Math/Color.h>
#include <string>
#include <vector>
#include <memory>

// string은 화면에 그릴 문자.
// vector는 한 프레임의 그리기 명령 목록.
// frame은 화면 한 장 분량의 데이터.

namespace Craft
{
	// unique_ptr이라 전방 선언.
	class ScreenBuffer;
	class CRAFT_API Renderer
	{
		struct Frame
		{
			Frame(int bufferCount);
			~Frame();

			void Clear(const Vector2& screenSize);
			// 배열용 unique_ptr.
			// 고정으로 못 쓰기때문에 힙에 만듦.
			std::unique_ptr<CHAR_INFO[]> charInfoArray;

			std::unique_ptr<int[]> sortingOrderArray;
		};

		struct RenderCommand
		{
			// Actor가 직접 화면을 그리는 것이 아니라
			// Renderer에 명령 전달.
			// sortingOrder로 큰 명령을 나중에 그려서 화면 앞쪽에 보이게 함.

			std::string image;
			Vector2 position = Vector2::Zero;
			Color color = Color::White;
			int sortingOrder = -1;
		};
		
		public:
		// 생성자 - Renderer 전역 접근 설정과 콘솔 커서 숨김 처리.
		// 소멸자 - 콘솔 커서를 원래 상태로 되돌림.
			Renderer(const Vector2& screenSize);
			~Renderer();
		
			void Submit(
				const std::string& image,
				const Vector2& position,
				Color color = Color::White,
				int sortingOrder = 0

			);

			// 한 프레임에 모인 명령을 실제 화면에 그림.
			// Engine이 매 프레임 마지막에 호출.
			// Get 함수는 Actor가 현재 Renderer 객체에 접근해서 Submit 호출 가능하게 함.
			
			void Draw();
			static Renderer& Get();

	private:
		// 이전 화면을 지우고,
		// 모아둔 명령을 실제로 그림.
		// 완성된 화면을 사용자에게 보여줌.

		void Clear();
		void DrawRenderQueue();
		void Present();
		const ScreenBuffer* const GetCurrentBuffer() const;
			
		// 인스턴스는 현재 Renderer 객체의 주소를 저장.
		// Renderer::Get() 접근하게 함.
		static Renderer* instance;
		// 이번 프레임에 그릴 명령들을 순서대로 모아두는 배열.
		std::vector<RenderCommand> renderQueue;

		Vector2 screenSize;

		std::unique_ptr<Frame> frame;

		std::unique_ptr<ScreenBuffer> screenBufferArray[2];

		int currentBufferIndex = 0;
	
	};
}