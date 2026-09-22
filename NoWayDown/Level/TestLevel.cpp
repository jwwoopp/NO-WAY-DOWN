#include "TestLevel.h"
#include <cmath>
#include <Renderer/Renderer.h>
#include <Input/Input.h>
#include <Engine/Engine.h>
#include "../Map/MapLoader.h"
#include <algorithm>

void TestLevel::OnInitialized()
{
	Craft::Level::OnInitialized();
	// SpawnActor<TestActor>();
    mapLoaded = MapLoader::Load("Assets/Floor1.txt", mapData);

    if (mapLoaded)
    {
        camera.x = static_cast<float>(mapData.playerStart.x) + 0.5f;
        camera.y = static_cast<float>(mapData.playerStart.y) + 0.5f;
    }
}

// 카메라 위치를 빼고 회전하고 가로세로 배율 적용.
Craft::Vector2 TestLevel::WorldToScreen(float x, float y) const
{
    float relativeX = x - camera.x;
    float relativeY = y - camera.y;

    float cosine = std::cos(camera.angle);
    float sine = std::sin(camera.angle);

    float rotatedX = relativeX * cosine - relativeY * sine;
    float rotatedY = relativeX * sine + relativeY * cosine;

    float centerX = (Craft::Engine::Get().GetWidth() - 1) * 0.5f;
    float centerY = (Craft::Engine::Get().GetHeight() - 1) * 0.5f;

    int screenX = static_cast<int>(
        std::round(centerX + rotatedX * camera.scaleX));

    int screenY = static_cast<int>(
        std::round(centerY + rotatedY * camera.scaleY));

    return Craft::Vector2(screenX, screenY);
}

void TestLevel::Tick(float deltaTime)
{

    Craft::Level::Tick(deltaTime);

    float horizontal = 0.0f;
    float vertical = 0.0f;

    if (Craft::Input::Get().GetKey('A')) horizontal -= 1.0f;
    if (Craft::Input::Get().GetKey('D')) horizontal += 1.0f;
    if (Craft::Input::Get().GetKey('W')) vertical -= 1.0f;
    if (Craft::Input::Get().GetKey('S')) vertical += 1.0f;

    float length = std::sqrt(
        horizontal * horizontal + vertical * vertical);

    if (length > 0.0f)
    {
        float distance = 3.0f * deltaTime;
        horizontal = horizontal / length * distance;
        vertical = vertical / length * distance;

        float cosine = std::cos(camera.angle);
        float sine = std::sin(camera.angle);

        camera.x += horizontal * cosine + vertical * sine;
        camera.y += -horizontal * sine + vertical * cosine;
    }

    if (Craft::Input::Get().GetKeyDown(VK_ESCAPE))
        Craft::Engine::Get().Quit();
}

void TestLevel::Draw()
{
    if (!mapLoaded)
    {
        Craft::Renderer::Get().Submit("Floor1.txt load failed - ESC: Quit",
            Craft::Vector2(0, 0), Craft::Color::Red, 10);
        return;
    }

    // The existing map supplies the floor dimensions; no per-floor draw code.
    auto a = WorldToScreen(0.0f, 0.0f);
    auto b = WorldToScreen(static_cast<float>(mapData.width), 0.0f);
    auto c = WorldToScreen(static_cast<float>(mapData.width), static_cast<float>(mapData.height));
    auto d = WorldToScreen(0.0f, static_cast<float>(mapData.height));
    DrawTriangle(a, b, c, Craft::Color::Floor, 0);
    DrawTriangle(a, c, d, Craft::Color::Floor, 0);

    // At the fixed 45-degree angle, smaller x+y is farther away.
    for (int depth = 0; depth < mapData.width + mapData.height - 1; ++depth)
    {
        for (int y = 0; y < mapData.height; ++y)
        {
            int x = depth - y;
            if (x < 0 || x >= mapData.width)
                continue;

            TileType tile = mapData.tiles[y * mapData.width + x];
            if (tile == TileType::Wall)
                DrawBox(static_cast<float>(x), static_cast<float>(y), 1.0f, 1.0f, 6);
            else if (tile == TileType::ClosedDoor || tile == TileType::Exit)
                Craft::Renderer::Get().Submit(tile == TileType::Exit ? "E" : "D",
                    WorldToScreen(x + 0.5f, y + 0.5f), Craft::Color::Yellow, 1);
        }
    }

    // Preview marker only, not a playable actor. Keep it visible behind walls.
    Craft::Renderer::Get().Submit(
        "P", WorldToScreen(mapData.playerStart.x + 0.5f, mapData.playerStart.y + 0.5f),
        Craft::Color::Green, 3);
    Craft::Renderer::Get().Submit("Floor1 preview | WASD: Camera | ESC: Quit",
        Craft::Vector2(0, 0), Craft::Color::White, 10);
}

void TestLevel::DrawLine(
    const Craft::Vector2& start,
    const Craft::Vector2& end)
{
    int dx = end.x - start.x;
    int dy = end.y - start.y;

    int steps = std::abs(dx);
    if (std::abs(dy) > steps)
        steps = std::abs(dy);

    for (int index = 0; index <= steps; ++index)
    {
        float ratio = steps == 0
            ? 0.0f
            : static_cast<float>(index) / steps;

        int x = static_cast<int>(std::round(start.x + dx * ratio));
        int y = static_cast<int>(std::round(start.y + dy * ratio));

        Craft::Renderer::Get().Submit(
            "#", Craft::Vector2(x, y),
            Craft::Color::Cyan, 2);
    }
}

void TestLevel::DrawTriangle(
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
    int minX = (std::max)(0, (std::min)({a.x, b.x, c.x}));
    int maxX = (std::min)(width - 1, (std::max)({a.x, b.x, c.x}));
    int minY = (std::max)(0, (std::min)({a.y, b.y, c.y}));
    int maxY = (std::min)(height - 1, (std::max)({a.y, b.y, c.y}));
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

void TestLevel::DrawBox(
    float x, float y,
    float width, float depth,
    int height)
{
    Craft::Vector2 bottom[4] =
    {
        WorldToScreen(x, y),
        WorldToScreen(x + width, y),
        WorldToScreen(x + width, y + depth),
        WorldToScreen(x, y + depth)
    };

    Craft::Vector2 top[4];

    for (int index = 0; index < 4; ++index)
    {
        top[index] = bottom[index];
        top[index].y -= height;
    }

    // 오른쪽 면.
    DrawTriangle(bottom[1], bottom[2], top[2], Craft::Color::Blue);
    DrawTriangle(bottom[1], top[2], top[1], Craft::Color::Blue);

    // 왼쪽 면.
    DrawTriangle(bottom[2], bottom[3], top[3], Craft::Color::Cyan);
    DrawTriangle(bottom[2], top[3], top[2], Craft::Color::Cyan);

    // 윗면.
    DrawTriangle(top[0], top[1], top[2], Craft::Color::White);
    DrawTriangle(top[0], top[2], top[3], Craft::Color::White);

}
