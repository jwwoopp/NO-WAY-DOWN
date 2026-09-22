#pragma once

#include <Math/Vector2.h>
#include <Math/Color.h>
#include <string>

// 5x7 격자 블록 글꼴.
// 콘솔 기본 글자는 타이틀로 쓰기엔 너무 작아서,
// 글자 하나를 색 블록 여러 칸으로 크게 그림.
namespace BlockFont
{
	// pixelWidth / pixelHeight는 격자 한 칸이 차지하는 화면 열 / 행 수.
	// 콘솔 셀이 세로로 길어서 pixelWidth를 pixelHeight의 두 배로 주면
	// 격자 한 칸이 화면에서 정사각형으로 보임.
	void Draw(
		const std::string& text,
		int screenX, int screenY,
		int pixelWidth, int pixelHeight,
		Craft::Color color,
		int sortingOrder = 10);

	// 가운데 정렬에 쓰려고 그려질 가로 폭을 미리 구함.
	int MeasureWidth(const std::string& text, int pixelWidth);

	// 글자 한 줄의 세로 높이.
	int MeasureHeight(int pixelHeight);
}
