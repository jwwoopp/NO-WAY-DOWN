#pragma once

#include <Actor/Actor.h>

class TestActor : public Craft::Actor 
{
public:
	// TestActor가 만들어질 때
	// 부모 Actor 생성자에 화면 문자, 시작 위치, 색상 전달하기 위핸 생성자.
	TestActor();
	virtual void Tick(float deltaTime) override;
};