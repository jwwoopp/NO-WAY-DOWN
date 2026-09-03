#pragma once

#include <Actor/Actor.h>

class TestActor : public Craft::Actor 
{
public:
	virtual void Tick(float deltaTime) override;
};