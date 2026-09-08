#pragma once

#include <Math/Vector2.h>

class AStarNode
{
public:
	AStarNode(
		const Craft::Vector2& position,
		AStarNode* parent = nullptr)
		: position(position), parent(parent)
	{
	}

	Craft::Vector2 position;

	float gCost = 0.0f;
	float hCost = 0.0f;
	float fCost = 0.0f;

	AStarNode* parent = nullptr;

};