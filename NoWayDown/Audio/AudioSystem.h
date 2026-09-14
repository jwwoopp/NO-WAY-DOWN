#pragma once

// 게임 소리 재생기.
//
// CraftEngine이 아니라 게임 쪽에 둔 이유:
// 소리 목록 자체가 게임 내용이고, 엔진에 넣으면 Includes/의 헤더 사본까지
// 같이 관리해야 해서 실수하기 쉬움.
//
// 내부는 mciSendString을 씀. PlaySound는 한 번에 하나만 재생돼서
// 배경음과 효과음을 동시에 낼 수 없기 때문.
// 소리마다 별칭을 하나씩 열어두면 서로 겹쳐서 재생됨.
class AudioSystem
{
public:
	enum class SoundId
	{
		MainTheme,        // maingame.wav        배경음
		FloorTransition,  // floor_transition.wav 층 이동
		Danger,           // danger.wav          좀비가 플레이어를 발견
		Shot,             // shot.wav            총 발사
		ZombieStep,       // zombiestep.wav      가까운 좀비의 발소리
		DoorKnock,        // doorknock.wav       좀비가 문을 두드림
		DoorBreak,        // doorshout.wav       문이 부서짐
		Move,             // move.wav            플레이어 발소리
		GameOver,         // gameover.wav        사망

		Count
	};

	static AudioSystem& Get();

	// 파일을 열어 별칭을 잡아둠. 실패해도 게임은 그대로 돌아감.
	void Initialize();
	void Shutdown();

	// 한 번 재생. 같은 소리가 재생 중이면 처음부터 다시 시작함.
	// 소리마다 정해둔 최소 간격이 지나지 않았으면 무시함.
	void Play(SoundId id);

	// 배경음. 이미 같은 소리가 흐르고 있으면 아무 것도 하지 않음.
	// 층이 바뀔 때마다 배경음이 처음으로 되돌아가는 걸 막으려는 것.
	void PlayLoop(SoundId id);

	void Stop(SoundId id);
	void StopAll();

	// 배경음이 끝났으면 다시 틀고, 재생 간격 타이머를 줄임.
	// 매 프레임 불러도 되지만 실제 확인은 가끔만 함.
	void Update(float deltaTime);

private:
	AudioSystem() = default;

	struct Entry
	{
		const char* fileName = nullptr;
		// 0이면 파일 전체를 재생. 그 외에는 앞에서 이만큼(밀리초)만 재생함.
		// 발소리처럼 긴 파일에서 한 번의 소리만 떼어 쓰기 위한 값.
		int playMilliseconds = 0;
		// 이 시간이 지나기 전에는 다시 재생하지 않음. 소리가 뭉개지는 걸 막음.
		float minimumInterval = 0.0f;
		bool opened = false;
		float cooldown = 0.0f;
	};

	Entry& GetEntry(SoundId id);
	bool IsPlaying(SoundId id) const;

	Entry entries[static_cast<int>(SoundId::Count)];
	bool initialized = false;

	// 지금 반복 재생 중인 배경음. 없으면 Count.
	SoundId loopingSound = SoundId::Count;
	// 배경음이 끝났는지 확인하는 주기. 매 프레임 확인할 필요가 없음.
	float loopCheckTimer = 0.0f;
};
