#include "IsometricRenderer.h"
#include <Engine/Engine.h>
#include <cmath>
#include <Renderer/Renderer.h>
#include <algorithm>
#include <string>
#include "CharacterSpriteData.h"

void IsometricRenderer::DrawCharacter(float worldX, float worldY,
    CharacterSprite sprite, int sortingOrder, int verticalOffset,
    int direction, int frame) const
{
    // 16 x 16 logical pixels. The larger silhouette remains readable when
    // the map is zoomed out, while the half-block renderer keeps each pixel square.
    // 위에서 똑바로 내려다본 모습이라 머리 위와 어깨만 보이고 다리는 몸에 가려짐.
    // 다리를 아래에 붙이면 그건 정면 뷰가 되므로 넣지 않음.
    // 한 칸이 한 색이라 끈이나 손가락은 표현하지 않음. '.'은 투명.

    // 생존자: 가운데 머리, 그 둘레를 배낭이 감싸고, 좌우 대칭으로 팔이 뻗음.
    // 대칭이라 "똑바로 선 사람"으로 읽힘.
    // H 머리 위, P 배낭, A 어깨와 팔.
    static constexpr const char* survivorShape[] = {
        "...." "...." "...." "....",
        "...." "OHHH" "HOO." "....",
        "...." "OHHH" "HHHO" "....",
        "...." "OHHH" "HHHO" "....",
        "..O." "AAHH" "HHAA" ".O..",
        "..OA" "AHHH" "HHAA" "AO..",
        "..OA" "APPR" "RPAA" "AO..",
        "..OA" "APPP" "PPAA" "AO..",
        "..OA" "APPP" "PPAA" "AO..",
        "..OA" "APPA" "PPAA" "AO..",
        "..OA" "AAAA" "AAAA" "AO..",
        "...O" "ASSS" "SSAO" "....",
        "...." ".OAA" "AAO." "....",
        "...." ".OAA" "AAO." "....",
        "...." ".OO." "OO.." "....",
        "...." "...." "...." "...."
    };

    // 좀비: 구부정해서 머리가 앞쪽 아래로 쏠리고 팔이 좌우 비대칭으로 벌어짐.
    // 색을 지워도 이 실루엣만으로 생존자와 구분되는 게 목표.
    // Z 살, H 숙인 머리, C 뻗은 팔, R 피.
    static constexpr const char* zombieShape[] = {
        "...." "...." "...." "....",
        "...." "OZZR" "RZO." "....",
        "...O" "ZZZZ" "ZZZO" "....",
        "..OC" "ZZZZ" "ZZZO" "....",
        "..OC" "ZZZZ" "ZZZO" "O...",
        ".OCC" "ZZHH" "HHZC" "CO..",
        ".OCZ" "ZHHH" "HHZZ" "CCO.",
        ".OZZ" "HHHH" "HZZC" "CCO.",
        "..OZ" "ZZSS" "SSZZ" "CO..",
        "..OC" "ZZZR" "RZZC" "O...",
        "...O" "ZZZZ" "ZZO." "....",
        "...." "OZZZ" "ZZO." "....",
        "...." ".OZZ" "ZZO." "....",
        "...." ".OZZ" "ZZO." "....",
        "...." ".OO." "OO.." "....",
        "...." "...." "...." "...."
    };

    direction = (std::max)(0, (std::min)(
        NoWayDown::CharacterDirectionCount - 1, direction));
    frame = (std::max)(0, (std::min)(
        NoWayDown::CharacterFrameCount - 1, frame));

    // 4-direction x 3-frame sheets converted to the indexed console format.
    // The older symbolic silhouettes above remain as a source-level fallback.
    const int sheetIndex =
        direction * NoWayDown::CharacterFrameCount + frame;
    const char* const* shape = sprite == CharacterSprite::Zombie
        ? NoWayDown::ZombieSpriteFrames[sheetIndex]
        : NoWayDown::SurvivorSpriteFrames[sheetIndex];
    const int spriteWidth = NoWayDown::CharacterSpriteWidth;
    const int spriteHeight = NoWayDown::CharacterSpriteHeight;

    // 위에서 보는 시점이라 발밑이 아니라 타일 중심에 맞춤.
    auto center = WorldToScreen(worldX + 0.5f, worldY + 0.5f);

    // 타일 크기가 창 크기에 따라 달라지므로 스프라이트 한 칸도 같이 커짐.
    // 사람이 타일의 대략 3/4를 차지하게 맞춤.
    int pixelWidth = static_cast<int>(GetScaleX() * 0.88f / spriteWidth);
    int pixelHeight = static_cast<int>(GetScaleY() * 0.88f / spriteHeight);

    if (pixelWidth < 1)
    {
        pixelWidth = 1;
    }

    if (pixelHeight < 1)
    {
        pixelHeight = 1;
    }

    const int originX = center.x - (spriteWidth * pixelWidth) / 2;
    const int originY = center.y - (spriteHeight * pixelHeight) / 2
        + verticalOffset;

    for (int y = 0; y < spriteHeight; ++y)
    {
        for (int x = 0; x < spriteWidth; ++x)
        {
            const char part = shape[y][x];

            if (part == '.')
            {
                continue;
            }

            Craft::Color color = Craft::Color::ClothLight;

            switch (part)
            {
            case 'A': color = Craft::Color::ClothLight; break;
            case 'P': color = Craft::Color::Hair;       break;
            case 'O': color = Craft::Color::Ink;        break;
            case 'S': color = Craft::Color::Shadow;     break;
            case 'h': color = Craft::Color::Hair;       break;
            case 'Z': color = Craft::Color::RotSkin;    break;
            case 'C': color = Craft::Color::RotSkin;    break;
            case 'R': color = Craft::Color::Danger;     break;
            case 'H':
                // 생존자 머리는 몸과 같은 크림색.
                // 측면 인트로 캐릭터도 크림 몸에 회록색 배낭이라 같은 사람으로 보임.
                color = Craft::Color::Skin;
                break;
            default: break;
            }

            // 스프라이트 한 칸을 pixelWidth x pixelHeight 덩어리로 그림.
            const std::string block(pixelWidth, ' ');

            for (int line = 0; line < pixelHeight; ++line)
            {
                Craft::Renderer::Get().Submit(block,
                    Craft::Vector2(
                        originX + x * pixelWidth,
                        originY + y * pixelHeight + line),
                    static_cast<Craft::Color>(
                        static_cast<WORD>(color) << 4),
                    sortingOrder);
            }
        }
    }
}

void IsometricRenderer::SetCameraPosition(
    const Craft::Vector2& position)
{
    // 타일의 왼쪽 위가 아니라 중심을 바라봄.
    cameraX = static_cast<float>(position.x) + 0.5f;
    cameraY = static_cast<float>(position.y) + 0.5f;
}

void IsometricRenderer::SetCameraCenter(float x, float y)
{
    cameraX = x;
    cameraY = y;
}

void IsometricRenderer::RotateCamera(float deltaAngle)
{
	angle += deltaAngle;

    const float twoPi = 6.28318530718f;

    while (angle >= twoPi)
    {
        angle -= twoPi;
    }

    while (angle < 0.0f)
    {
        angle += twoPi;
	}
}

void IsometricRenderer::SmoothRotateTo(float targetAngle, float maxStep)
{
	const float twoPi = 6.28318530718f;

	while (targetAngle >= twoPi)
	{
		targetAngle -= twoPi;
	}

	while (targetAngle < 0.0f)
	{
		targetAngle += twoPi;
	}

	// -π~π 범위의 최단 회전량을 구함.
	float delta = targetAngle - angle;

	if (delta > 3.14159265359f)
	{
		delta -= twoPi;
	}
	else if (delta < -3.14159265359f)
	{
		delta += twoPi;
	}

	if (maxStep <= 0.0f || std::abs(delta) <= maxStep)
	{
		angle = targetAngle;
		return;
	}

	angle += delta > 0.0f ? maxStep : -maxStep;

	while (angle >= twoPi)
	{
		angle -= twoPi;
	}

	while (angle < 0.0f)
	{
		angle += twoPi;
	}
}

void IsometricRenderer::SetCameraFollow(
    float targetX, float targetY,
    float mapWidth, float mapHeight)
{
    const float screenWidth =
        static_cast<float>(Craft::Engine::Get().GetWidth());
    const float screenHeight =
        static_cast<float>(Craft::Engine::Get().GetHeight());

    // 회전하면 화면 모서리가 더 멀리 보이므로, 회전된 화면의 외접 사각형을 사용함.
    const float halfScreenX = screenWidth * 0.5f;
    const float halfScreenY = screenHeight * 0.5f;
    const float cosine = std::cos(angle);
    const float sine = std::sin(angle);
    const float halfViewWidth =
        (std::abs(cosine) * halfScreenX / GetScaleX())
        + (std::abs(sine) * halfScreenY / GetScaleY());
    const float halfViewHeight =
        (std::abs(sine) * halfScreenX / GetScaleX())
        + (std::abs(cosine) * halfScreenY / GetScaleY());

    cameraX = targetX;
    cameraY = targetY;

    // 맵이 화면보다 좁은 축은 가운데 고정.
    // 넓은 축은 가장자리에서 멈춰 맵 바깥의 빈 공간을 비추지 않게 함.
    if (mapWidth <= halfViewWidth * 2.0f)
    {
        cameraX = mapWidth * 0.5f;
    }
    else if (cameraX < halfViewWidth)
    {
        cameraX = halfViewWidth;
    }
    else if (cameraX > mapWidth - halfViewWidth)
    {
        cameraX = mapWidth - halfViewWidth;
    }

    if (mapHeight <= halfViewHeight * 2.0f)
    {
        cameraY = mapHeight * 0.5f;
    }
    else if (cameraY < halfViewHeight)
    {
        cameraY = halfViewHeight;
    }
    else if (cameraY > mapHeight - halfViewHeight)
    {
        cameraY = mapHeight - halfViewHeight;
    }
}

float IsometricRenderer::GetScaleX() const
{
    float scale = Craft::Engine::Get().GetWidth()
        / static_cast<float>(visibleTilesX);

    // 타일 경계가 칸 경계에 딱 떨어지도록 정수로 내림.
    scale = std::floor(scale);

    return scale < 2.0f ? 2.0f : scale;
}

float IsometricRenderer::GetScaleY() const
{
    // Half-block pixels are approximately square: use equal X/Y scale.
    return GetScaleX();
}

Craft::Vector2 IsometricRenderer::WorldToScreen(
    float x, float y) const
{
    const float scaleX = GetScaleX();
    const float scaleY = GetScaleY();

    float relativeX = x - cameraX;
    float relativeY = y - cameraY;

    // 카메라가 도는 것처럼 보이게 월드의 상대 위치만 회전함.
    const float cosine = std::cos(angle);
    const float sine = std::sin(angle);
    const float rotatedX = relativeX * cosine - relativeY * sine;
    const float rotatedY = relativeX * sine + relativeY * cosine;

    // 타일 경계가 화면 칸 경계에 정확히 떨어지도록 폭을 그대로 절반으로 나눔.
    float centerX = Craft::Engine::Get().GetWidth() * 0.5f;
    float centerY = Craft::Engine::Get().GetHeight() * 0.5f;

    return Craft::Vector2(
        static_cast<int>(std::round(centerX + rotatedX * scaleX)),
        static_cast<int>(std::round(centerY + rotatedY * scaleY))
        + screenOffsetY
    );
}

Craft::Vector2 IsometricRenderer::ScreenToWorld(const Craft::Vector2& pixel) const
{
    const float cosine = std::cos(angle);
    const float sine = std::sin(angle);
    const float screenX =
        (pixel.x - Craft::Engine::Get().GetWidth() * 0.5f) / GetScaleX();
    const float screenY =
        (pixel.y - screenOffsetY
            - Craft::Engine::Get().GetHeight() * 0.5f) / GetScaleY();

    // WorldToScreen의 회전을 역으로 되돌려 클릭 좌표를 맵 칸으로 변환함.
    const float worldX = screenX * cosine + screenY * sine;
    const float worldY = -screenX * sine + screenY * cosine;

    return Craft::Vector2(
        static_cast<int>(std::floor(cameraX + worldX)),
        static_cast<int>(std::floor(cameraY + worldY)));
}

void IsometricRenderer::DrawRect(
    int x0, int y0, int x1, int y1,
    Craft::Color color, int sortingOrder)
{
    int width = Craft::Engine::Get().GetWidth();
    int height = Craft::Engine::Get().GetHeight();

    int minX = (std::max)(0, x0);
    int maxX = (std::min)(width, x1);
    int minY = (std::max)(0, y0);
    int maxY = (std::min)(height, y1);

    for (int y = minY; y < maxY; ++y)
    {
        for (int x = minX; x < maxX; ++x)
        {
            Craft::Renderer::Get().Submit(
                " ", Craft::Vector2(x, y),
                static_cast<Craft::Color>(
                    static_cast<WORD>(color) << 4), sortingOrder);
        }
    }
}

void IsometricRenderer::DrawTriangle(
    const Craft::Vector2& a,
    const Craft::Vector2& b,
    const Craft::Vector2& c,
    Craft::Color color, int sortingOrder)
{
    int area = (b.x - a.x) * (c.y - a.y)
        - (b.y - a.y) * (c.x - a.x);

    if (area == 0)
        return;

    int width = Craft::Engine::Get().GetWidth();
    int height = Craft::Engine::Get().GetHeight();

    // Only test pixels inside the triangle's clipped bounding rectangle.
    int minX = (std::max)(0, (std::min)({ a.x, b.x, c.x }));
    int maxX = (std::min)(width - 1, (std::max)({ a.x, b.x, c.x }));
    int minY = (std::max)(0, (std::min)({ a.y, b.y, c.y }));
    int maxY = (std::min)(height - 1, (std::max)({ a.y, b.y, c.y }));
    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            int e0 = (b.x - a.x) * (y - a.y)
                - (b.y - a.y) * (x - a.x);
            int e1 = (c.x - b.x) * (y - b.y)
                - (c.y - b.y) * (x - b.x);
            int e2 = (a.x - c.x) * (y - c.y)
                - (a.y - c.y) * (x - c.x);

            bool inside = (e0 >= 0 && e1 >= 0 && e2 >= 0)
                || (e0 <= 0 && e1 <= 0 && e2 <= 0);

            if (inside)
            {
                Craft::Renderer::Get().Submit(
                    " ", Craft::Vector2(x, y),
                    static_cast<Craft::Color>(
                        static_cast<WORD>(color) << 4), sortingOrder);
            }
        }
    }
}

void IsometricRenderer::DrawQuad(
    const Craft::Vector2& a,
    const Craft::Vector2& b,
    const Craft::Vector2& c,
    const Craft::Vector2& d,
    Craft::Color color,
    int sortingOrder)
{
    DrawTriangle(a, b, c, color, sortingOrder);
    DrawTriangle(a, c, d, color, sortingOrder);
}

void IsometricRenderer::DrawTile(
    float x, float y, Craft::Color color, int sortingOrder)
{
    const Craft::Vector2 top = WorldToScreen(x, y);
    const Craft::Vector2 right = WorldToScreen(x + 1.0f, y);
    const Craft::Vector2 bottom = WorldToScreen(x + 1.0f, y + 1.0f);
    const Craft::Vector2 left = WorldToScreen(x, y + 1.0f);

    DrawQuad(top, right, bottom, left, color, sortingOrder);
}

void IsometricRenderer::DrawBox(
    float x, float y,
    float width, float depth,
    int height,
    Craft::Color topColor,
    Craft::Color frontColor,
    bool bottomExposed, bool rightExposed)
{
    const Craft::Vector2 top = WorldToScreen(x, y);
    const Craft::Vector2 right = WorldToScreen(x + width, y);
    const Craft::Vector2 bottom = WorldToScreen(x + width, y + depth);
    const Craft::Vector2 left = WorldToScreen(x, y + depth);

    const Craft::Vector2 topRight = right;
    const Craft::Vector2 bottomRight = bottom;

    // 높이는 화면 아래쪽으로 내리는 얇은 전면으로 표현함.
    const Craft::Vector2 bottomLeftFront(
        left.x, left.y + height);
    const Craft::Vector2 bottomRightFront(
        bottomRight.x, bottomRight.y + height);

    // 앞면: 자기 타일 아래쪽으로 height행만큼 흘러내림.
    // 아래 칸의 벽이 나중에 그려지면서 이 앞면을 덮으므로,
    // 벽이 세로로 이어질 때는 맨 아랫줄에만 앞면이 남음.
    if (bottomExposed)
    {
        // Short contact shadow, then the thin front face. Drawing only:
        // collision still uses the original map tile.
        DrawQuad(left, bottomRight, bottomRightFront, bottomLeftFront,
            frontColor, 1);
        DrawQuad(
            Craft::Vector2(left.x + 1, left.y + height),
            Craft::Vector2(bottomRight.x + 1, bottomRight.y + height),
            Craft::Vector2(bottomRight.x + 1, bottomRight.y + height + 1),
            Craft::Vector2(left.x + 1, left.y + height + 1),
            Craft::Color::Shadow, 1);
    }

    // 윗면: 자기 타일 자리를 그대로 차지함.
    // 위 칸을 침범하지 않으므로 그 칸에 선 캐릭터가 벽에 올라탄 것처럼 보이지 않음.
    DrawQuad(top, topRight, bottomRight, left, topColor, 2);

    // Dark bevel on exposed edges only: joined walls stay one solid mass.
    if (rightExposed)
        DrawQuad(
            Craft::Vector2(right.x - 1, right.y),
            right,
            bottomRight,
            Craft::Vector2(bottomRight.x - 1, bottomRight.y),
            frontColor, 3);
    if (bottomExposed)
        DrawQuad(
            Craft::Vector2(left.x, left.y - 1),
            Craft::Vector2(bottomRight.x, bottomRight.y - 1),
            bottomRight,
            left,
            frontColor, 3);
}

void IsometricRenderer::DrawFacingMarker(float worldX, float worldY,
    int directionX, int directionY,
    Craft::Color color, int sortingOrder) const
{
    if (directionX == 0 && directionY == 0)
    {
        return;
    }

    const auto center = WorldToScreen(worldX + 0.5f, worldY + 0.5f);

    // 월드 방향을 현재 카메라 각도만큼 같이 돌려 화면에서 올바른 방향을 가리킴.
    const float distanceX = directionX * GetScaleX() / 3.0f;
    const float distanceY = directionY * GetScaleY() / 3.0f;
    const float cosine = std::cos(angle);
    const float sine = std::sin(angle);
    const int endX = center.x + static_cast<int>(
        std::round(distanceX * cosine - distanceY * sine));
    const int endY = center.y + static_cast<int>(
        std::round(distanceX * sine + distanceY * cosine));

    const int screenWidth = Craft::Engine::Get().GetWidth();
    const int screenHeight = Craft::Engine::Get().GetHeight();

    const Craft::Color blockColor =
        static_cast<Craft::Color>(static_cast<WORD>(color) << 4);

    const int steps = (std::max)(std::abs(endX - center.x),
        std::abs(endY - center.y));

    for (int step = 0; step <= steps; ++step)
    {
        const float ratio = steps == 0
            ? 0.0f : static_cast<float>(step) / steps;
        const int x = center.x + static_cast<int>(
            std::round((endX - center.x) * ratio));
        const int y = center.y + static_cast<int>(
            std::round((endY - center.y) * ratio));

        if (x < 0 || y < 0 || x >= screenWidth || y >= screenHeight)
        {
            continue;
        }

        Craft::Renderer::Get().Submit(
            " ", Craft::Vector2(x, y), blockColor, sortingOrder);
    }
}
