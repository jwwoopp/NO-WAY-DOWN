#include "Renderer.h"
#include "ScreenBuffer.h"
#include <cassert>
#include <iostream>
#include <Windows.h>


namespace Craft
{
	// ------------- Frame --------------- //
	// Renderer - 클래스.
	// Frame - 구조체.
	// Frame() - 생성자.
	Renderer::Frame::Frame(int bufferCount)
	{
		charInfoArray = std::make_unique<CHAR_INFO[]>(bufferCount);
		sortingOrderArray = std::make_unique<int[]>(bufferCount);
	}

	Renderer::Frame::~Frame()
	{
	}

	void Renderer::Frame::Clear(const Vector2& screenSize)
	{
		const int width = screenSize.x;
		const int height = screenSize.y;

		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				// 화면은 2차원인데 배열은 1차원.
				// y * width로 몇 줄 내려갈지.
				// + x로 그 줄에서 몇 칸 갈지 정함.
				const int index = (y * width) + x;

				// 초기화 값들.
				CHAR_INFO& info = charInfoArray[index];
				// 빈 칸.
				info.Char.AsciiChar = ' ';
				// 색 없음.
				info.Attributes = 0;
				// 아직 아무것도 안 그려짐.
				sortingOrderArray[index] = -1;
			}
		}
	}
	// ------------- Frame --------------- //d

	// 헤더에서의 아래 코드는 변수 존재 알려주는 선언.
	// static Renderer* instance;
	// 하지만 실제 메모리 공간은 아직 생성 X.
	// 그래서 cpp에서 한 번 정의.
	Renderer* Renderer::instance = nullptr;

	// instance에는 현재 Renderer 객체의 주소 삽입.
	// 생성 전에는 아무 객체도 없어서 nullptr

	
	Renderer::Renderer(const Vector2& screenSize)
		: screenSize(screenSize)
	{
		assert(!instance && "instance should be null");
		instance = this;

		const int bufferCount = screenSize.x * screenSize.y;
		frame = std::make_unique<Frame>(bufferCount);

		frame->Clear(screenSize);

		// 화면 두 장 만들기.
		screenBufferArray[0] = std::make_unique<ScreenBuffer>(screenSize);
		screenBufferArray[0]->Clear();

		screenBufferArray[1] = std::make_unique<ScreenBuffer>(screenSize);
		screenBufferArray[1]->Clear();

		SetConsoleActiveScreenBuffer(screenBufferArray[0]->GetBuffer());
	}

	Renderer::~Renderer()
	{
		instance = nullptr;

		// 원래 화면으로 복구.
		SetConsoleActiveScreenBuffer(GetStdHandle(STD_OUTPUT_HANDLE));
	}

	void Renderer::Submit(const std::string& image, const Vector2& position, Color color, int sortingOrder)
	{
		// 전달받은 정보를 RenderCommand 하나로 묶음.
		RenderCommand command;
		command.image = image;
		command.position = position;
		command.color = color;
		command.sortingOrder = sortingOrder;

		// renderQueue 끝에 추가.
		renderQueue.emplace_back(command);
	}

	void Renderer::Draw()
	{
		// 이전 화면 지우기.
		Clear();
		// 예약된 문자 그리기.
		DrawRenderQueue();
		// 완성된 화면 표시.
		Present();
	}

	Renderer& Renderer::Get()
	{
		// Renderer가 만들어지지 않았다면 instance는 nullptr
		// assert로 실행 멈춤.
		// 정상이라면 pointer에 *를 붙여 현재 Renderer 객체를 참조로 반환.
		assert(instance && "instance should not be null");
		return *instance;
	}

	void Renderer::Clear()
	{
		// 메모리 배열 1000칸을 공백으로.
		frame->Clear(screenSize);

		// 콘솔 버퍼 자체를 공백으로.
		GetCurrentBuffer()->Clear();
	}

	void Renderer::DrawRenderQueue()
	{
		for (const RenderCommand& command : renderQueue)
		{
			// 1. 화면 밖인지 검사.
			// cout일 때는 화면 밖이면 Windows가 알아서 무시했지만
			// 이제는 배열에 직접 쓰기 때문에 직접 막아야 함.
			if (command.image.empty())
			{
				continue;
			}

			if (command.position.y < 0
				|| command.position.y >= screenSize.y)
			{
				continue;
			}

			const int length = static_cast<int>(command.image.length());

			const int startX = command.position.x;

			const int endX = startX + length - 1;

			if (endX < 0 || startX >= screenSize.x)
			{
				continue;
			}

			// 2. 잘라내기(클리핑)
			// 글자가 반만 걸친 경우를 처리.

			const int visibleStart = startX < 0 ? 0 : startX;
			// 조건 ? A : B - 조건이 참이면 A, 참이 아니면 B.
			// startX가 -2라 0부터 그림.
			const int visibleEnd
				= endX >= screenSize.x ? screenSize.x - 1 : endX;

			for (int x = visibleStart; x <= visibleEnd; ++x)
			{
				const int sourceIndex = x - startX;

				const int index = (command.position.y * screenSize.x) + x;

				// 3. 순서 비교; 후 기록.
				// 이미 그려진 게 더 높으면 건너 뜀.
				// 벽(0) 위에 플레이어(5)는 그려지고, 플레이어 위에 벽은 안그려짐.

				if (frame->sortingOrderArray[index] > command.sortingOrder)
				{
					continue;
				}

				frame->charInfoArray[index].Char.AsciiChar
					= command.image[sourceIndex];

				frame->charInfoArray[index].Attributes
					= static_cast<DWORD>(command.color);

				frame->sortingOrderArray[index] = command.sortingOrder;
			}
		}
		// 4. 마지막에 한 번 그리기.
		GetCurrentBuffer()->Draw(frame->charInfoArray.get());

		renderQueue.clear();

		SetConsoleTextAttribute(
			GetCurrentBuffer()->GetBuffer(),
			static_cast<DWORD>(Color::White)
		);
	}

	void Renderer::Present()
	{
		SetConsoleActiveScreenBuffer(GetCurrentBuffer()->GetBuffer());

		// 0과 1을 뒤집는 식.
		currentBufferIndex = 1 - currentBufferIndex;
		// if (currentBufferIndex == 0) currentBufferIndex = 1;
		// else currentBufferIndex = 0;
	}

	const ScreenBuffer* const Renderer::GetCurrentBuffer() const
	{
		// 진짜 주소만 꺼냄.
		return screenBufferArray[currentBufferIndex].get();
	}

}