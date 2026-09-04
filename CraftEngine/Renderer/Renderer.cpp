#include "Renderer.h"
#include <cassert>
#include <iostream>
#include <Windows.h>

namespace Craft
{
	// 헤더에서의 아래 코드는 변수 존재 알려주는 선언.
	// static Renderer* instance;
	// 하지만 실제 메모리 공간은 아직 생성 X.
	// 그래서 cpp에서 한 번 정의.
	Renderer* Renderer::instance = nullptr;

	// instance에는 현재 Renderer 객체의 주소 삽입.
	// 생성 전에는 아무 객체도 없어서 nullptr

	
	Renderer::Renderer()
	{
		// 이미 Renderer가 존재하면 assert로 중복 생성 방지.
		// 현재 객체 주소를 instance 저장.
		assert(!instance && "instance should be null");
		instance = this;

		// Windows 콘솔 깜빡이는 입력 커서 숨김.
		CONSOLE_CURSOR_INFO info;
		GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

		info.bVisible = FALSE;
		SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
	}

	Renderer::~Renderer()
	{
		// Renderer가 사라지므로 nullptr로 되돌림.
		instance = nullptr;

		CONSOLE_CURSOR_INFO info;
		GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

		info.bVisible = TRUE;
		SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

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
		system("cls");
	}

	void Renderer::DrawRenderQueue()
	{
		// renderQueue에 저장된 그리기 명령을 하나씩 꺼내는 반복문.
		for (const RenderCommand& command : renderQueue)
		{
			// 콘솔 화면을 다루기 위한 handle 추가.
			HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

			// (어느 콘솔을 움직일지, 문자를 그릴 xy 위치) 
			// Vector2에 COORD 변환 연산자를 만들었기 때문에 position을 그대로 전달.
			SetConsoleCursorPosition(handle, command.position);
			SetConsoleTextAttribute(
				handle,
				static_cast<WORD>(command.color)
			);

			std::cout << command.image;

			SetConsoleTextAttribute(
				handle,
				static_cast<WORD>(Color::White)
			);
		}

		renderQueue.clear();
	}

	void Renderer::Present()
	{
	}

}