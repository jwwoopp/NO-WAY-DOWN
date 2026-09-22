#include "SceneArt.h"
#include <Renderer/Renderer.h>
#include <Engine/Engine.h>
#include <Windows.h>
#include <string>

using namespace Craft;

namespace
{
	// 공통 기호.
	// F 어두운 문틀과 그림자, W 나무 문짝, P 패널 홈, K 금속 손잡이,
	// B 사람 몸(크림), G 배낭, E 눈, L 신발, R 피, '.' 투명.

	const char* closedDoorShape[] =
	{
		"FFFFFFFFFFFF",
		"FWWWWWWWWWWF",
		"FWPPPPPPPPWF",
		"FWPWWWWWWPWF",
		"FWPWWWWWWPWF",
		"FWPWWWWWWPWF",
		"FWPPPPPPPPWF",
		"FWWWWWWWWWWF",
		"FWWWWWWWWKWF",
		"FWPPPPPPPPWF",
		"FWPWWWWWWPWF",
		"FWPWWWWWWPWF",
		"FWPWWWWWWPWF",
		"FWPPPPPPPPWF",
		"FWWWWWWWWWWF",
		"FFFFFFFFFFFF"
	};

	// 문짝이 왼쪽으로 열려 있고 가운데는 어둠으로 뚫려 있음.
	const char* openDoorShape[] =
	{
		"FFFFFFFFFFFF",
		"FWWF.......F",
		"FWPF.......F",
		"FWPF.......F",
		"FWPF.......F",
		"FWPF.......F",
		"FWWF.......F",
		"FWKF.......F",
		"FWWF.......F",
		"FWPF.......F",
		"FWPF.......F",
		"FWPF.......F",
		"FWPF.......F",
		"FWWF.......F",
		"FWWF.......F",
		"FFFFFFFFFFFF"
	};

	// 오른쪽을 보고 선 사람. 배낭은 등 쪽이라 왼편에 붙음.
	const char* standingShape[] =
	{
		"...BBB..",
		"..BBBBB.",
		"..BBBBE.",
		"..BBBB..",
		".GGBBBB.",
		".GGBBBB.",
		".GGBBBB.",
		"..BBBB..",
		"..BB.B..",
		"..BB.B..",
		"..LL.LL."
	};

	// 쓰러진 사람. 머리가 오른쪽, 다리가 왼쪽.
	const char* fallenShape[] =
	{
		"..........",
		"....GGBBB.",
		".LLBBBBBBB",
		".LLBBBBBB.",
		"....RR...."
	};

	// 벽에 붙은 배전반.
	const char* panelPropShape[] =
	{
		"FFFFFFFF",
		"FPPPPPPF",
		"FPWWWWPF",
		"FPWKKWPF",
		"FPWWWWPF",
		"FPPPPPPF",
		"FFFFFFFF",
		"........"
	};

	// 둥근 밸브 손잡이.
	const char* valvePropShape[] =
	{
		"...FF...",
		"..FWWF..",
		".FW..WF.",
		"FW.KK.WF",
		"FW.KK.WF",
		".FW..WF.",
		"..FWWF..",
		"...FF..."
	};

	Color ResolveColor(char part)
	{
		switch (part)
		{
		case 'F': return Color::WallFront;   // 어두운 문틀
		case 'W': return Color::FloorGrime;  // 나무 문짝
		case 'P': return Color::FloorBase;   // 패널 홈
		case 'K': return Color::ClothDark;   // 금속
		case 'B': return Color::ClothLight;  // 사람 몸
		case 'G': return Color::ClothDark;   // 배낭
		case 'E': return Color::WallFront;   // 눈
		case 'L': return Color::WallFront;   // 신발
		case 'R': return Color::Danger;      // 피
		default:  return Color::ClothLight;
		}
	}

	void DrawShape(
		const char* const* shape, int rowCount, int columnCount,
		int x, int y, int sortingOrder)
	{
		for (int row = 0; row < rowCount; ++row)
		{
			for (int column = 0; column < columnCount; ++column)
			{
				const char part = shape[row][column];

				if (part == '.')
				{
					continue;
				}

				SceneArt::FillRect(
					x + column, y + row, 1, 1,
					ResolveColor(part), sortingOrder);
			}
		}
	}
}

void SceneArt::FillRect(
	int x, int y, int width, int height,
	Color color, int sortingOrder)
{
	if (width <= 0 || height <= 0)
	{
		return;
	}

	// 이 게임은 공백 문자의 배경색으로 그림을 그림.
	const Color blockColor =
		static_cast<Color>(static_cast<WORD>(color) << 4);

	const std::string row(width, ' ');

	for (int line = 0; line < height; ++line)
	{
		Renderer::Get().SubmitCells(
			row, Vector2(x, y + line), blockColor, sortingOrder);
	}
}

void SceneArt::DrawFloorLine(int y, Color color, int sortingOrder)
{
	FillRect(0, y, Engine::Get().GetWidth(), 1, color, sortingOrder);
}

void SceneArt::DrawDoor(int x, int bottomY, bool open, int sortingOrder)
{
	// bottomY가 문이 바닥에 닿는 줄.
	const int top = bottomY - DoorHeight() + 1;

	DrawShape(
		open ? openDoorShape : closedDoorShape,
		DoorHeight(), DoorWidth(), x, top, sortingOrder);
}

void SceneArt::DrawStandingPerson(int x, int bottomY, int sortingOrder)
{
	DrawShape(
		standingShape, PersonHeight(), PersonWidth(),
		x, bottomY - PersonHeight() + 1, sortingOrder);
}

void SceneArt::DrawFallenPerson(int x, int bottomY, int sortingOrder)
{
	DrawShape(
		fallenShape, FallenHeight(), FallenWidth(),
		x, bottomY - FallenHeight() + 1, sortingOrder);
}

void SceneArt::DrawPanelProp(int x, int bottomY, int sortingOrder)
{
	DrawShape(panelPropShape, 8, 8, x, bottomY - 7, sortingOrder);
}

void SceneArt::DrawValveProp(int x, int bottomY, int sortingOrder)
{
	DrawShape(valvePropShape, 8, 8, x, bottomY - 7, sortingOrder);
}

int SceneArt::DoorWidth() { return 12; }
int SceneArt::DoorHeight() { return 16; }
int SceneArt::PersonWidth() { return 8; }
int SceneArt::PersonHeight() { return 11; }
int SceneArt::FallenWidth() { return 10; }
int SceneArt::FallenHeight() { return 5; }
