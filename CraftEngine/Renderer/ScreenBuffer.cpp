#include "ScreenBuffer.h"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

namespace Craft
{
	namespace
	{
		// 게임 팔레트.
		// Math/Color.h의 인덱스 이름과 순서가 정확히 같아야 함.
		const COLORREF gamePalette[16] =
		{
			RGB(0x14, 0x15, 0x0F),   // 0  Ink        배경
			RGB(0x2A, 0x2E, 0x28),   // 1  WallFront  벽의 밝은 띠
			RGB(0x3A, 0x44, 0x38),   // 2  FloorBase  바닥 어두운 얼룩
			RGB(0x6E, 0x7A, 0x66),   // 3  FloorGrime 바닥 기본
			RGB(0x8A, 0x94, 0x80),   // 4  ClothDark  바닥 밝은 얼룩
			RGB(0xDA, 0xD5, 0xBE),   // 5  ClothLight 생존자 밝은 몸
			RGB(0xC6, 0xBF, 0xA6),   // 6  WallTop    계단 줄무늬, 타이틀 글자
			RGB(0xB3, 0xA8, 0x8C),   // 7  DoorTop    문 나무
			RGB(0x8F, 0x85, 0x6C),   // 8  DoorEdge   문 판자 줄
			RGB(0x7A, 0x70, 0x57),   // 9  DoorFront  문틀
			RGB(0x4E, 0x5C, 0x48),   // 10 RotSkin    좀비 살
			RGB(0x96, 0xA0, 0x88),   // 11 Stairs     출구 계단
			RGB(0xA0, 0x3A, 0x30),   // 12 Danger     피, 경고
			RGB(0xA8, 0x98, 0x78),   // 13 Skin       배낭, 피부, HUD 글자
			RGB(0x1E, 0x21, 0x1C),   // 14 Shadow     벽 본체
			RGB(0x6B, 0x57, 0x44)    // 15 Hair       신발, 나무 갈색
		};

		// "38;2;R;G;B"(글자색) 또는 "48;2;R;G;B"(배경색) 형태로 이어 붙임.
		void AppendTrueColor(
			std::string& out, const char* prefix, COLORREF color)
		{
			out += "\x1b[";
			out += prefix;
			out += ";2;";
			out += std::to_string(GetRValue(color));
			out += ';';
			out += std::to_string(GetGValue(color));
			out += ';';
			out += std::to_string(GetBValue(color));
			out += 'm';
		}

		// 64 x 24 logical pixels become 64 x 12 terminal cells.
		// First square: solid fill. Second: single-pixel checkerboard.
		// Third: circle and diagonal, useful for checking pixel proportions.
		int PreviewPixel(int x, int y)
		{
			if (y < 4 || y >= 20) return 0;
			if (x >= 2 && x < 18) return 5;
			if (x >= 24 && x < 40) return ((x + y) % 2 == 0) ? 5 : 12;
			if (x >= 46 && x < 62)
			{
				const int dx = 2 * (x - 46) - 15;
				const int dy = 2 * (y - 4) - 15;
				if (x - 46 == y - 4) return 12;
				if (dx * dx + dy * dy <= 225) return 5;
			}
			return 0;
		}

		void DrawHalfBlockPreview(HANDLE buffer, int width, int height)
		{
			const int left = width > 64 ? (width - 64) / 2 : 0;
			const int top = height > 18 ? (height - 18) / 2 : 0;
			const std::string title = "HALF BLOCK TEST | ESC: EXIT";
			const std::string labels = "  SOLID                 CHECKER               CIRCLE / LINE";
			static std::string out;
			out.clear();
			int last = -1;
			for (int y = 0; y < height; ++y)
			{
				out += "\x1b[" + std::to_string(y + 1) + ";1H";
				for (int x = 0; x < width; ++x)
				{
					const int px = x - left;
					const int row = y - top - 3;
					const bool sample = px >= 0 && px < 64 && row >= 0 && row < 12;
					int upper = 0;
					int lower = 0;
					char letter = ' ';
					if (sample)
					{
						upper = PreviewPixel(px, row * 2);
						lower = PreviewPixel(px, row * 2 + 1);
					}
					else if (px >= 0)
					{
						const std::string* text = y == top ? &title : (y == top + 16 ? &labels : nullptr);
						if (text && px < static_cast<int>(text->size()))
						{
							letter = (*text)[px];
							upper = 5;
						}
					}
					const int colors = upper | (lower << 4);
					if (colors != last)
					{
						AppendTrueColor(out, "38", gamePalette[upper]);
						AppendTrueColor(out, "48", gamePalette[lower]);
						last = colors;
					}
					if (sample) out += "\xE2\x96\x80"; // U+2580, upper half block.
					else out += letter;
				}
			}
			out += "\x1b[0m";
			// Explicit UTF-8 conversion + Unicode output: no global code-page change.
			const int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
				out.data(), static_cast<int>(out.size()), nullptr, 0);
			if (count <= 0) return;
			static std::vector<wchar_t> wide;
			wide.resize(count);
			if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
				out.data(), static_cast<int>(out.size()), wide.data(), count)) return;
			DWORD written = 0;
			WriteConsoleW(buffer, wide.data(), static_cast<DWORD>(count), &written, nullptr);
		}
	}
	ScreenBuffer::ScreenBuffer(const Vector2& screenSize)
		// 초기화 리스트.
		// size를 screenSize로 만들면서 시작.
		: size(screenSize)
	{
		wchar_t previewFlag[2] = {};
		halfBlockPreview = GetEnvironmentVariableW(L"NWD_HALF_BLOCK_TEST", previewFlag, 2) == 1
			&& previewFlag[0] == L'1';
		// 게임 전용 콘솔 버퍼를 만들어 렌더링함.
		// Windows Terminal의 autoHideWindow 설정은 터미널 쪽에서 끄고,
		// 여기서는 기존처럼 독립 버퍼를 사용해 게임 화면 배치를 유지함.
		buffer = CreateConsoleScreenBuffer(
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr,
			CONSOLE_TEXTMODE_BUFFER,
			nullptr
		);
		ownsBuffer = true;

		// 콘솔 API 실패를 assert로 잡으면 Debug 빌드가 abort()로 죽음.
		if (buffer == INVALID_HANDLE_VALUE)
		{
			buffer = nullptr;
			ownsBuffer = false;
			return;
		}

		// 이 폰트와 모니터에서 실제로 만들 수 있는 최대 콘솔 창 크기.
		// 요청한 창이 이보다 크면 SetConsoleWindowInfo가 반드시 실패하므로 미리 줄임.
		COORD largest = GetLargestConsoleWindowSize(buffer);

		SHORT windowWidth = static_cast<SHORT>(size.x);
		SHORT windowHeight = static_cast<SHORT>(size.y);

		if (largest.X > 0 && windowWidth > largest.X)
			windowWidth = largest.X;

		if (largest.Y > 0 && windowHeight > largest.Y)
			windowHeight = largest.Y;

		if (windowWidth < 1)
			windowWidth = 1;

		if (windowHeight < 1)
			windowHeight = 1;

		// 창은 버퍼보다 클 수 없음.
		// 버퍼 크기를 바꾸기 전에 창을 최소로 만들어 두면
		// 기존 콘솔이 어떤 크기였든 다음 호출이 막히지 않음.
		SMALL_RECT minimalRect = { 0, 0, 0, 0 };
		SetConsoleWindowInfo(buffer, TRUE, &minimalRect);

		// 버퍼는 반드시 요청한 크기를 유지해야 함.
		// Renderer의 CHAR_INFO 배열이 size 기준이라 여기서 줄이면 줄이 어긋남.
		// 창만 작아지는 건 아래쪽이 화면 밖으로 나갈 뿐 그림은 안 깨짐.
		SetConsoleScreenBufferSize(buffer, size);

		SMALL_RECT rect = {};
		rect.Left = 0;
		rect.Top = 0;
		// -1이 붙는 이유는 좌표가 0에서 시작하기 때문.
		rect.Right = windowWidth - 1;
		rect.Bottom = windowHeight - 1;
		SetConsoleWindowInfo(buffer, TRUE, &rect);

		// 콘솔 16색 팔레트를 게임 팔레트로 교체함.
		// Color.h의 인덱스 이름과 순서가 정확히 같아야 함.
		// conhost에서만 적용됨. Windows Terminal은 자기 배색을 쓰므로 무시함.
		CONSOLE_SCREEN_BUFFER_INFOEX paletteInfo = {};
		paletteInfo.cbSize = sizeof(paletteInfo);

		if (GetConsoleScreenBufferInfoEx(buffer, &paletteInfo))
		{
			for (int index = 0; index < 16; ++index)
			{
				paletteInfo.ColorTable[index] = gamePalette[index];
			}

			// SetConsoleScreenBufferInfoEx가 창을 한 행, 한 열씩 줄이는
			// 오래된 버그가 있어서 미리 1씩 더해 둠.
			paletteInfo.srWindow.Right += 1;
			paletteInfo.srWindow.Bottom += 1;

			SetConsoleScreenBufferInfoEx(buffer, &paletteInfo);
		}

		// ANSI 이스케이프를 해석하도록 켬.
		// 이게 켜져야 Draw에서 24비트 RGB 색을 직접 지정할 수 있고,
		// 그러면 콘솔 16색 팔레트를 무시하는 Windows Terminal에서도
		// 게임이 의도한 색이 그대로 나옴.
		// 줄 끝 자동 개행은 마지막 칸을 쓸 때 화면이 밀리므로 끔.
		DWORD consoleMode = 0;

		if (GetConsoleMode(buffer, &consoleMode))
		{
			consoleMode |= ENABLE_PROCESSED_OUTPUT;
			consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			consoleMode &= ~ENABLE_WRAP_AT_EOL_OUTPUT;

			SetConsoleMode(buffer, consoleMode);

			// 터미널에게 게임 크기로 맞춰 달라고 요청함.
			//
			// 위쪽의 SetConsoleWindowInfo는 conhost에서만 통하고
			// Windows Terminal은 그 API를 무시함.
			// 반면 이 VT 시퀀스(CSI 8 ; 행 ; 열 t)는 Windows Terminal이 받아줌.
			// 이게 없으면 창이 게임보다 작을 때 화면이 잘리고,
			// 사용자가 settings.json이나 글꼴 크기를 직접 고쳐야 했음.
			// 환경을 사람이 맞추는 게 아니라 프로그램이 맞추는 쪽이 맞음.
			//
			// 창이 게임보다 큰 경우는 Draw에서 가운데 정렬 + 여백 채우기로 이미 처리함.
			// 요청이 거부돼도 그 경로로 떨어지므로 실패를 확인하지 않음.
			// 터미널에게 게임 크기로 맞춰 달라고 요청함.
			//
			// 위쪽의 SetConsoleWindowInfo는 conhost에서만 통하고
			// Windows Terminal은 그 API를 무시함.
			// 반면 이 VT 시퀀스(CSI 8 ; 행 ; 열 t)는 Windows Terminal이 받아줌.
			// 이게 없으면 창이 게임보다 작을 때 화면이 잘리고,
			// 사용자가 settings.json이나 글꼴 크기를 직접 고쳐야 했음.
			// 환경을 사람이 맞추는 게 아니라 프로그램이 맞추는 쪽이 맞음.
			//
			// 창이 게임보다 큰 경우는 Draw에서 가운데 정렬 + 여백 채우기로 처리함.
			// 요청이 거부돼도 그 경로로 떨어지므로 실패를 확인하지 않음.
			std::string resizeRequest = "\x1b[8;";
			resizeRequest += std::to_string(size.y);
			resizeRequest += ';';
			resizeRequest += std::to_string(size.x);
			resizeRequest += 't';

			DWORD resizeWritten = 0;

			WriteConsoleA(
				buffer,
				resizeRequest.data(),
				static_cast<DWORD>(resizeRequest.size()),
				&resizeWritten,
				nullptr);
		}

		// 커서 설정은 화면 한 장마다 따로.
		// Renderer에서 껐지만 커서를 또 끔.
		CONSOLE_CURSOR_INFO info = {};

		if (GetConsoleCursorInfo(buffer, &info))
		{
			info.bVisible = FALSE;
			SetConsoleCursorInfo(buffer, &info);
		}
	}

	ScreenBuffer::~ScreenBuffer()
	{
		if (ownsBuffer && buffer)
		{
			// Windows에서 빌려온 건 돌려줘야 함.
			// 안 그러면 프로그램 끝날 때까지 자원 물려있음.
			// = fclose
			CloseHandle(buffer);
		}
	}

	void ScreenBuffer::Clear() const
	{
		if (!buffer)
		{
			return;
		}

		// 콘솔 버퍼가 요청한 size보다 넓거나 높게 잡힐 수 있음.
		// 게임은 size 영역만 그리므로 그 바깥은 아무도 안 건드려서
		// 이전 화면 내용이 그대로 남음.
		// 그래서 실제 버퍼 크기를 물어보고 전체를 비움.
		CONSOLE_SCREEN_BUFFER_INFO info = {};
		DWORD cellCount =
			static_cast<DWORD>(size.x) * static_cast<DWORD>(size.y);

		if (GetConsoleScreenBufferInfo(buffer, &info))
		{
			cellCount = static_cast<DWORD>(info.dwSize.X)
				* static_cast<DWORD>(info.dwSize.Y);
		}

		DWORD writtenCount = 0;

		// 화면 지우기
		// (어느 화면을, 무엇으로, 몇 칸, 어디서부터, 실제로 몇 칸 채우는지 받을 곳).
		FillConsoleOutputCharacterA(
			buffer,
			' ',
			cellCount,
			Vector2::Zero,
			&writtenCount
		);

		// 글자만 지우면 배경색이 남음.
		// 이 게임은 배경색으로 그림을 그리므로 속성도 같이 되돌려야 함.
		FillConsoleOutputAttribute(
			buffer,
			0,
			cellCount,
			Vector2::Zero,
			&writtenCount
		);
	}

	// CHAR_INFO* 앞에 const.
	// charInfo 앞에 const.
	// const 두 개가 붙어서 가리키는 내용과 포인터 자체.
	// 변경 불가, 다른데 못가리키게 함.
	void ScreenBuffer::Draw(const CHAR_INFO* const charInfo) const
	{
		if (!buffer)
		{
			return;
		}

		// CHAR_INFO의 속성 바이트 대신 ANSI 이스케이프로 색을 직접 지정함.
		// 속성 방식은 콘솔 16색 팔레트에 묶여 있어서
		// 팔레트를 무시하는 Windows Terminal에서는 색이 전혀 달라짐.
		// 24비트 RGB로 내보내면 어느 터미널에서든 같은 색이 나옴.

		// 실제 콘솔 창 크기를 매 프레임 확인함. 창을 도중에 늘려도 따라감.
		CONSOLE_SCREEN_BUFFER_INFO consoleInfo = {};
		int realWidth = size.x;
		int realHeight = size.y;

		if (GetConsoleScreenBufferInfo(buffer, &consoleInfo))
		{
			const int visibleWidth =
				consoleInfo.srWindow.Right - consoleInfo.srWindow.Left + 1;
			const int visibleHeight =
				consoleInfo.srWindow.Bottom - consoleInfo.srWindow.Top + 1;

			if (visibleWidth > 0 && visibleHeight > 0)
			{
				realWidth = visibleWidth;
				realHeight = visibleHeight;
			}
		}

		// 뷰포트가 게임 화면보다 크면 그만큼 더 그려야 함.
		// 여기서 게임 크기로 깎으면 바깥 영역을 아무도 안 건드려서
		// 이전 프레임 내용이 오른쪽에 긴 띠로 남음.
		// 배열 범위는 아래 루프에서 칸마다 검사하므로 넘칠 일이 없음.
		if (realWidth < 1)
		{
			realWidth = 1;
		}

		if (realHeight < 1)
		{
			realHeight = 1;
		}

		if (halfBlockPreview)
		{
			DrawHalfBlockPreview(buffer, realWidth, realHeight);
			return;
		}

		// 게임 화면을 창 한가운데 놓고 남는 가장자리는 배경색으로 채움.
		// 이렇게 해야 화면이 잘린 것이 아니라 여백으로 보임.
		int offsetX = (realWidth - size.x) / 2;
		int offsetY = (realHeight - size.y) / 2;

		if (offsetX < 0)
		{
			offsetX = 0;
		}

		if (offsetY < 0)
		{
			offsetY = 0;
		}

		// 프레임마다 새로 할당하지 않도록 한 번 만든 문자열을 계속 재사용함.
		// 게임 루프는 단일 스레드라 이렇게 두어도 안전함.
		static std::string out;

		out.clear();

		if (out.capacity() == 0)
		{
			out.reserve(256 * 1024);
		}

		// 직전 칸과 색이 같으면 이스케이프를 생략함.
		// 이 게임은 큰 단색 덩어리가 많아서 대부분의 칸이 생략됨.
		int lastAttributes = -1;

		for (int y = 0; y < realHeight; ++y)
		{
			// 줄마다 커서를 직접 옮김. 자동 개행에 기대지 않음.
			out += "\x1b[";
			out += std::to_string(y + 1);
			out += ";1H";

			const int sourceY = y - offsetY;
			const bool insideRow = sourceY >= 0 && sourceY < size.y;

			for (int x = 0; x < realWidth; ++x)
			{
				const int sourceX = x - offsetX;

				// 게임 화면 바깥은 배경색 한 가지로 채움.
				int attributes = 0;
				char character = ' ';
				bool halfBlock = false;

				if (insideRow && sourceX >= 0 && sourceX < size.x)
				{
					// Two logical rows form one physical terminal cell.
					const CHAR_INFO& upper = charInfo[(sourceY * 2) * size.x + sourceX];
					const CHAR_INFO& lower = charInfo[(sourceY * 2 + 1) * size.x + sourceX];
					const auto isText = [](const CHAR_INFO& pixel)
					{
						return pixel.Char.AsciiChar != ' ' && pixel.Char.AsciiChar != '\0';
					};
					// Native debug text still occupies a complete terminal cell.
					// Pixel art uses the background palette index of each logical row.
					if (isText(upper) || isText(lower))
					{
						const CHAR_INFO& text = isText(upper) ? upper : lower;
						attributes = text.Attributes & 0xFF;
						character = text.Char.AsciiChar;
					}
					else
					{
						const int topColor = (upper.Attributes >> 4) & 0x0F;
						const int bottomColor = (lower.Attributes >> 4) & 0x0F;
						attributes = topColor | (bottomColor << 4);
						halfBlock = true;
					}
				}

				if (attributes != lastAttributes)
				{
					AppendTrueColor(
						out, "38", gamePalette[attributes & 0x0F]);
					AppendTrueColor(
						out, "48", gamePalette[(attributes >> 4) & 0x0F]);

					lastAttributes = attributes;
				}

				if (halfBlock) out += "\xE2\x96\x80";
				else out += (character == '\0') ? ' ' : character;
			}
		}

		// 색 설정을 원래대로 되돌려 두어야
		// 게임이 끝난 뒤 콘솔에 남는 글자가 이상한 색으로 나오지 않음.
		out += "\x1b[0m";

		// Same Unicode output path as the verified half-block preview.
		const int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			out.data(), static_cast<int>(out.size()), nullptr, 0);
		if (count <= 0) return;
		static std::vector<wchar_t> wide;
		wide.resize(count);
		if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			out.data(), static_cast<int>(out.size()), wide.data(), count)) return;
		DWORD writtenCount = 0;
		WriteConsoleW(buffer, wide.data(), static_cast<DWORD>(count), &writtenCount, nullptr);
	}

}
