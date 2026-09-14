#pragma once

#include <Level/Level.h>
#include <Game/RunState.h>

#include <memory>

// 게임이 끝났을 때 보여주는 결과 화면.
// 탈출 성공과 사망을 서로 다른 문구와 색으로 구분함.
class EndingLevel : public Craft::Level
{
	TYPE_DECLARATIONS(EndingLevel, Level)

public:
	EndingLevel(const std::shared_ptr<RunState>& newRunState);

	virtual void Tick(float deltaTime) override;
	virtual void Draw() override;

private:
	void StartNewRun();

	std::shared_ptr<RunState> runState;

	// 결과를 만들 때의 값을 그대로 들고 있음.
	// 재시작하면서 runState가 초기화돼도 화면 문구가 흔들리지 않게 하려는 것.
	bool cleared = false;
	int reachedFloor = 1;

	float blinkTimer = 0.0f;
};
