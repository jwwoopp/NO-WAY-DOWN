#pragma once

#include <Level/Level.h>
#include <Math/Vector2.h>
#include "../Map/MapData.h"
#include "../Renderer/IsometricRenderer.h"

class TestLevel : public Craft::Level
{
public:
	virtual void OnInitialized() override;
	void Tick(float deltaTime) override;
	void Draw() override;

private:
    Craft::Vector2 WorldToScreen(float x, float y) const;

    void DrawLine(
        const Craft::Vector2& start,
        const Craft::Vector2& end);

    void DrawTriangle(
        const Craft::Vector2& a,
        const Craft::Vector2& b,
        const Craft::Vector2& c,
        Craft::Color color, int sortingOrder = 2);

    void DrawBox(
        float x, float y,
        float width, float depth,
        int height);

    struct Camera
    {
        // 카메라 위치는 소수 단위로 움직여야하므로 float 저장.
        // 가로 세로 표시 배율.
        float x = 2.0f;
        float y = 2.0f;
        // 45도 라디안 표현.
        float angle = 0.785398f;
        float scaleX = 28.0f;
        float scaleY = 7.0f;
    };

    Camera camera;
    MapData mapData;
    bool mapLoaded = false;

};

