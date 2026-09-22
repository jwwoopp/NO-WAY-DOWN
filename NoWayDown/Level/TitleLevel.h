#pragma once

#include <Level/Level.h>
#include <Game/RunState.h>

#include <memory>

// 게임을 시작하기 전에 보여주는 타이틀 화면.
// 맵도 액터도 없고 글자만 그림.
class TitleLevel : public Craft::Level
{
	TYPE_DECLARATIONS(TitleLevel, Level)

public:
	TitleLevel(const std::shared_ptr<RunState>& newRunState);

	virtual void Tick(float deltaTime) override;
	virtual void Draw() override;

private:
	// 시작할 때 진행 상태를 처음부터 다시 만듦.
	void StartNewRun();

	// 문 앞에 서 있는지. 그때만 F로 들어갈 수 있음.
	bool IsNearDoor() const;

	// 문을 여는 짧은 연출을 시작함.
	void BeginDoorOpening();

	// 화면 크기가 창에 따라 달라지므로 배치를 매번 계산함.
	int FloorLineY() const;
	int StageWidth() const;
	int StageLeft() const;
	int DoorX() const;

	std::shared_ptr<RunState> runState;

	// 깜빡임에 쓰는 시간.
	float blinkTimer = 0.0f;
	float doorOpeningTimer = 0.0f;
	bool doorOpening = false;

	// 타이틀 화면에서 직접 움직이는 사람의 가로 위치.
	// 문까지 걸어가서 F로 열면 게임이 시작됨.
	// 이렇게 하면 이동과 문 여는 조작을 시작 전에 자연스럽게 익히게 됨.
	// 음수면 아직 초기 위치를 정하지 않았다는 뜻.
	float personX = -1.0f;
	float walkSpeed = 26.0f;
};
