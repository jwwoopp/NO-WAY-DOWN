#pragma once

#include <Math/Vector2.h>
#include <Math/Color.h>

// 그릴 캐릭터 종류.
// 색은 스프라이트 정의 안에 들어 있어서 따로 넘기지 않음.
enum class CharacterSprite
{
    Survivor,
    Zombie
};

class IsometricRenderer
{
public:
    // 그리기용 좌표는 실수. 칸 사이를 부드럽게 이동하는 모습을 표현하려는 것.
    void DrawCharacter(float worldX, float worldY,
        CharacterSprite sprite, int sortingOrder,
        int verticalOffset = 0,
        int direction = 0,
        int frame = 0) const;
    void SetCameraPosition(const Craft::Vector2& position);
    // 맵 전체가 한 화면에 들어올 때 쓰는 고정 카메라. 타일 보정 없이 그 좌표를 중심으로 봄.
    void SetCameraCenter(float x, float y);

    // 대상을 화면 중앙에 두되, 화면이 맵 바깥을 비추지 않도록 가장자리에서 멈춤.
    // 맵이 화면보다 작은 축은 그냥 가운데에 고정함.
    void SetCameraFollow(
        float targetX, float targetY,
        float mapWidth, float mapHeight);
    // 카메라를 월드 중심에서 회전함. 이동·충돌 좌표는 바뀌지 않음.
    void RotateCamera(float deltaAngle);
    // 목표 각도까지 최단 방향으로 조금씩 회전함.
    // 공간을 옮길 때 화면이 갑자기 뒤집히지 않게 하는 용도.
    void SmoothRotateTo(float targetAngle, float maxStep);
    float GetCameraAngle() const { return angle; }
    Craft::Vector2 WorldToScreen(float x, float y) const;
    Craft::Vector2 ScreenToWorld(const Craft::Vector2& pixel) const;

    // 화면 축에 나란한 사각형을 채움. x1, y1은 포함하지 않는 끝 좌표.
    void DrawRect(
        int x0, int y0, int x1, int y1,
        Craft::Color color,
        int sortingOrder = 2);

    void DrawTriangle(
        const Craft::Vector2& a,
        const Craft::Vector2& b,
        const Craft::Vector2& c,
        Craft::Color color,
        int sortingOrder = 2);

    // 회전된 타일을 채우기 위한 볼록 사각형.
    void DrawQuad(
        const Craft::Vector2& a,
        const Craft::Vector2& b,
        const Craft::Vector2& c,
        const Craft::Vector2& d,
        Craft::Color color,
        int sortingOrder = 2);

    // 윗면을 height만큼 위로 올리고 그 아래를 앞면으로 채워 높이를 표현함.
    void DrawBox(
        float x, float y,
        float width, float depth,
        int height,
        Craft::Color topColor = Craft::Color::WallTop,
        Craft::Color frontColor = Craft::Color::WallFront,
        bool bottomExposed = true, bool rightExposed = true);

    // 타일 한 칸을 통째로 채움. 높이가 없는 바닥 표시용.
    void DrawTile(float x, float y, Craft::Color color, int sortingOrder = 1);

    // 캐릭터가 바라보는 쪽으로 짧은 막대를 그림.
    // 들고 있는 총이자 방향 표시.
    // 4방향 스프라이트를 따로 만들지 않아도 어디를 보는지 바로 읽힘.
    void DrawFacingMarker(float worldX, float worldY,
        int directionX, int directionY,
        Craft::Color color, int sortingOrder) const;

private:
    float cameraX = 0.0f;
    float cameraY = 0.0f;
	// 카메라 회전각. 완전히 반듯하지 않게 약 8도 비틀어
	// 타일 모서리와 벽 두께가 살짝 보이도록 시작함.
	// Q/E로 계속 돌릴 수 있고, 이동·충돌 좌표는 바뀌지 않음.
	float angle = 0.14f;
    // 타일 크기를 고정하지 않고 화면 폭에서 계산함.
    // 창 크기가 달라져도 가로로 보이는 칸 수가 같아서 확대 비율이 유지되고,
    // 맵이 항상 화면보다 커서 바깥 여백이 보이지 않음.
    // 여백이 보이면 벽인지 맵 밖인지 구분이 안 돼서 화면이 지저분해짐.
    static const int visibleTilesX = 10;

    // Half-block drawing uses equal horizontal and vertical pixel scales.
    float GetScaleX() const;
    float GetScaleY() const;
    // 맵이 화면을 꽉 채우므로 아래로 밀 필요가 없음. HUD는 그 위에 겹쳐 그림.
    int screenOffsetY = 0;
};
