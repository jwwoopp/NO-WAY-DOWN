#include "AudioSystem.h"

#include <Windows.h>
#include <string>

// mciSendString이 들어 있는 라이브러리.
// vcxproj를 건드리지 않으려고 여기서 직접 연결함.
#pragma comment(lib, "winmm.lib")

namespace
{
	// 소리마다 별칭이 하나씩 필요함. 별칭이 같으면 서로를 끊어버림.
	const char* AliasOf(int index)
	{
		static char alias[32] = {};
		sprintf_s(alias, "nwd_sound_%d", index);
		return alias;
	}

	// MCI는 상대 경로를 잘 못 다룸.
	// 작업 디렉터리가 아니라 실행 파일 위치를 기준으로 잡아야
	// 어디서 실행하든 같은 파일을 찾음. BgmPlayer가 쓰던 방식과 같음.
	std::string ToFullPath(const char* fileName)
	{
		char exePath[MAX_PATH] = {};

		if (GetModuleFileNameA(nullptr, exePath, MAX_PATH) == 0)
		{
			return fileName;
		}

		std::string path = exePath;
		// GetModuleFileName은 역슬래시를 돌려주지만 둘 다 찾아둠.
		const size_t lastSlash = path.find_last_of("\\/");

		if (lastSlash == std::string::npos)
		{
			return fileName;
		}

		// Bin/x64/Debug/NoWayDown.exe 기준으로 Bin/x64/Assets/Audio.
		// 빌드 전에 xcopy가 Assets를 그쪽으로 복사해 둠.
		path.erase(lastSlash);
		path += "/../Assets/Audio/";
		path += fileName;

		return path;
	}
}

AudioSystem& AudioSystem::Get()
{
	static AudioSystem instance;
	return instance;
}

AudioSystem::Entry& AudioSystem::GetEntry(SoundId id)
{
	return entries[static_cast<int>(id)];
}

void AudioSystem::Initialize()
{
	if (initialized)
	{
		return;
	}

	initialized = true;

	// 파일 이름과 재생 규칙.
	// playMilliseconds는 파형을 실제로 훑어보고 정한 값임.
	// move.wav는 2.8초짜리 연속음이라 통째로 틀면 발소리가 뭉개지고,
	// zombiestep.wav는 앞 0.19초에만 소리가 들어 있음.
	GetEntry(SoundId::MainTheme) = { "MainGame_BGM.wav", 0, 0.0f };
	GetEntry(SoundId::FloorTransition) = { "floor_transition.wav", 0, 0.0f };
	GetEntry(SoundId::Danger) = { "danger.wav", 0, 1.5f };
	GetEntry(SoundId::Shot) = { "shot.wav", 1200, 0.2f };
	GetEntry(SoundId::ZombieStep) = { "zombiestep.wav", 260, 0.30f };
	GetEntry(SoundId::DoorKnock) = { "doorknock.wav", 0, 0.5f };
	GetEntry(SoundId::DoorBreak) = { "doorshout.wav", 0, 0.0f };
	GetEntry(SoundId::Move) = { "move.wav", 320, 0.22f };
	GetEntry(SoundId::GameOver) = { "gameover.wav", 0, 0.0f };

	for (int index = 0; index < static_cast<int>(SoundId::Count); ++index)
	{
		Entry& entry = entries[index];

		if (!entry.fileName)
		{
			continue;
		}

		const std::string path = ToFullPath(entry.fileName);

		std::string command = "open \"";
		command += path;
		command += "\" type waveaudio alias ";
		command += AliasOf(index);

		// 소리 파일이 없어도 게임은 그대로 돌아가야 하므로 실패를 무시함.
		entry.opened =
			mciSendStringA(command.c_str(), nullptr, 0, nullptr) == 0;
	}
}

void AudioSystem::Shutdown()
{
	if (!initialized)
	{
		return;
	}

	for (int index = 0; index < static_cast<int>(SoundId::Count); ++index)
	{
		if (!entries[index].opened)
		{
			continue;
		}

		std::string command = "close ";
		command += AliasOf(index);

		mciSendStringA(command.c_str(), nullptr, 0, nullptr);
		entries[index].opened = false;
	}

	initialized = false;
	loopingSound = SoundId::Count;
}

bool AudioSystem::IsPlaying(SoundId id) const
{
	const int index = static_cast<int>(id);

	if (!entries[index].opened)
	{
		return false;
	}

	char result[32] = {};

	std::string command = "status ";
	command += AliasOf(index);
	command += " mode";

	if (mciSendStringA(command.c_str(), result, sizeof(result), nullptr) != 0)
	{
		return false;
	}

	return strcmp(result, "playing") == 0;
}

void AudioSystem::Play(SoundId id)
{
	const int index = static_cast<int>(id);
	Entry& entry = entries[index];

	if (!entry.opened)
	{
		return;
	}

	// 간격이 지나지 않았으면 무시함.
	// 이게 없으면 발소리가 매 이동마다 처음으로 되감겨 뚝뚝 끊김.
	if (entry.cooldown > 0.0f)
	{
		return;
	}

	entry.cooldown = entry.minimumInterval;

	std::string command = "play ";
	command += AliasOf(index);
	command += " from 0";

	// 파일 앞부분만 떼어 쓰는 경우.
	if (entry.playMilliseconds > 0)
	{
		command += " to ";
		command += std::to_string(entry.playMilliseconds);
	}

	mciSendStringA(command.c_str(), nullptr, 0, nullptr);
}

void AudioSystem::PlayLoop(SoundId id)
{
	// 이미 이 소리가 배경음으로 흐르고 있으면 건드리지 않음.
	// 층이 바뀔 때마다 FloorLevel이 새로 만들어지는데,
	// 그때마다 배경음이 처음으로 되돌아가면 어색함.
	if (loopingSound == id && IsPlaying(id))
	{
		return;
	}

	loopingSound = id;

	Entry& entry = entries[static_cast<int>(id)];
	entry.cooldown = 0.0f;

	Play(id);
}

void AudioSystem::Stop(SoundId id)
{
	const int index = static_cast<int>(id);

	if (!entries[index].opened)
	{
		return;
	}

	if (loopingSound == id)
	{
		loopingSound = SoundId::Count;
	}

	std::string command = "stop ";
	command += AliasOf(index);

	mciSendStringA(command.c_str(), nullptr, 0, nullptr);
}

void AudioSystem::StopAll()
{
	loopingSound = SoundId::Count;

	for (int index = 0; index < static_cast<int>(SoundId::Count); ++index)
	{
		if (!entries[index].opened)
		{
			continue;
		}

		std::string command = "stop ";
		command += AliasOf(index);

		mciSendStringA(command.c_str(), nullptr, 0, nullptr);
	}
}

void AudioSystem::Update(float deltaTime)
{
	for (Entry& entry : entries)
	{
		if (entry.cooldown > 0.0f)
		{
			entry.cooldown -= deltaTime;
		}
	}

	if (loopingSound == SoundId::Count)
	{
		return;
	}

	// 배경음이 끝났는지 확인. MCI 질의는 싸지 않아서 가끔만 확인함.
	loopCheckTimer -= deltaTime;

	if (loopCheckTimer > 0.0f)
	{
		return;
	}

	loopCheckTimer = 0.5f;

	if (!IsPlaying(loopingSound))
	{
		Entry& entry = entries[static_cast<int>(loopingSound)];
		entry.cooldown = 0.0f;

		Play(loopingSound);
	}
}
