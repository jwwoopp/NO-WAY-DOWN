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

		// 두 장의 물리 버퍼를 번갈아 사용함.
		// 현재 화면을 지우는 동안 다른 버퍼가 보여서 깜빡임이 생기지 않음.
		const Vector2 consoleSize(screenSize.x, screenSize.y / 2);
		// 화면 버퍼는 한 장만 씀.
		// WriteConsole이 화면 전체를 한 번에 쓰므로 그 자체가 찢김 없는 블릿이고,
		// 두 장을 번갈아 쓰면 매 프레임 SetConsoleActiveScreenBuffer를 부르게 되는데
		// Windows Terminal이 그때마다 뷰포트를 다시 만드느라 갱신이 밀려서
		// 화면 일부가 이전 내용으로 남음. 잔상의 원인이 이것이었음.
		screenBufferArray[0] = std::make_unique<ScreenBuffer>(consoleSize);
		screenBufferArray[0]->Clear();

		// 시작할 때 한 번만 활성 화면으로 지정함.
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

	void Renderer::SubmitCells(const std::string& image, const Vector2& position,
		Color color, int sortingOrder)
	{
		Submit(image, Vector2(position.x, position.y * 2), color, sortingOrder);
		Submit(image, Vector2(position.x, position.y * 2 + 1), color, sortingOrder);
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

	bool Renderer::ConsoleToPixel(COORD cell, Vector2& pixel) const
	{
		CONSOLE_SCREEN_BUFFER_INFO info = {};
		if (!GetConsoleScreenBufferInfo(GetCurrentBuffer()->GetBuffer(), &info)) return false;
		const int width = info.srWindow.Right - info.srWindow.Left + 1;
		const int height = info.srWindow.Bottom - info.srWindow.Top + 1;
		const int offsetX = width > screenSize.x ? (width - screenSize.x) / 2 : 0;
		const int offsetY = height > screenSize.y / 2 ? (height - screenSize.y / 2) / 2 : 0;
		const int x = cell.X - info.srWindow.Left;
		const int y = cell.Y - info.srWindow.Top;
		if (x < 0 || y < 0 || x >= width || y >= height) return false;
		const int px = x - offsetX;
		const int py = (y - offsetY) * 2 + 1;
		if (px < 0 || py < 0 || px >= screenSize.x || py >= screenSize.y) return false;
		pixel = Vector2(px, py);
		return true;
	}

	void Renderer::CopyCurrentFrame(std::vector<CHAR_INFO>& out) const
	{
		const size_t cellCount =
			static_cast<size_t>(screenSize.x) * static_cast<size_t>(screenSize.y);

		out.resize(cellCount);

		// Draw는 매 프레임 시작에 frame을 지우므로,
		// Tick 중에 부르면 직전 프레임에 그려진 화면이 그대로 들어 있음.
		memcpy(
			out.data(),
			frame->charInfoArray.get(),
			cellCount * sizeof(CHAR_INFO));
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

		// 콘솔 버퍼는 여기서 비우지 않음.
		// Draw가 뷰포트 전 칸을 덮어쓰므로 비울 필요가 없고,
		// 매 프레임 비우면 지워진 순간이 화면에 노출돼 깜빡임이 생김.
		// 뷰포트 바깥 잔상은 Draw가 가장자리를 배경색으로 채워서 해결함.
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
		// 화면 한 장만 쓰므로 여기서 할 일이 없음.
		// DrawRenderQueue의 WriteConsole이 이미 화면에 다 쓴 상태임.
		//
		// 매 프레임 활성 화면을 바꾸던 이전 방식은 되살리지 말 것.
		// 깜빡임을 없애려던 것인데 Windows Terminal에서는 오히려
		// 갱신이 밀려서 잔상이 생김.
		// SetConsoleActiveScreenBuffer(GetCurrentBuffer()->GetBuffer());
		// currentBufferIndex = 1 - currentBufferIndex;
	}

	const ScreenBuffer* const Renderer::GetCurrentBuffer() const
	{
		// 진짜 주소만 꺼냄.
		return screenBufferArray[currentBufferIndex].get();
	}

}
