#pragma once

#include <Core/Core.h>
#include <Windows.h>

namespace Craft
{
	enum class CRAFT_API Color : WORD
	{
        Red = FOREGROUND_RED,
        Green = FOREGROUND_GREEN,
        Blue = FOREGROUND_BLUE,
        Yellow = Red | Green,
        Cyan = Green | Blue,
        Purple = Red | Blue,
        White = Red | Green | Blue,
        BrightWhite = White | FOREGROUND_INTENSITY,
        Floor = FOREGROUND_INTENSITY,

        // 게임 팔레트 이름.
        // 콘솔은 16색뿐이고 색 자체는 ScreenBuffer가
        // SetConsoleScreenBufferInfoEx로 지정함.
        // 아래 이름들은 위 이름들과 같은 인덱스를 가리키는 별칭임.
        // 즉 White와 DoorTop은 같은 7번 색이고, 어떤 RGB인지는 팔레트가 정함.
        Ink = 0,          // #111316 배경, 빈 공간
        WallFront = 1,    // #2A2D2F 벽 앞면
        FloorBase = 2,    // #45494A 바닥
        FloorGrime = 3,   // #6B6B63 바닥 얼룩
        ClothDark = 4,    // #8A8779 어두운 천, 바지
        ClothLight = 5,   // #B5AE98 밝은 천, 셔츠
        WallTop = 6,      // #D8CCAA 벽 윗면
        DoorTop = 7,      // #3E5B5B 문 윗면, 생존자 재킷
        DoorEdge = 8,     // #587878 문틀
        DoorFront = 9,    // #243A3C 문 앞면
        RotSkin = 10,     // #6C745C 좀비 피부
        Stairs = 11,      // #9A9D86 계단, 출구
        Danger = 12,      // #A94B45 피, 피격, 경고
        Skin = 13,        // #D6C6A4 피부, 구급상자
        Shadow = 14,      // #171B1E 그림자
        Hair = 15         // #7A5C4B 머리카락, 나무
    };
}