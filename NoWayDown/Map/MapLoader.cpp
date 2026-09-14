#include "MapLoader.h"
#include <cstdio>
#include <utility>

// fopen_s로 파일을 열고 fread로 내용을 읽는다.
// 파일 읽기 실패하면 false.
// 성공한 경우에는 반드시 fclose로 닫음.
bool MapLoader::Load(const std::string& filename, MapData& outData)
{
	FILE* file = nullptr;
	fopen_s(&file, filename.c_str(), "rt");

	if (!file)
	{
		return false;
	}

	// 파일 끝에서 크기를 확인한 뒤, 처음으로 돌아와 내용을 읽는 흐름.
	if (fseek(file, 0, SEEK_END) != 0)
	{
		fclose(file);
		return false;
	}

	long fileSize = ftell(file);
	rewind(file);

	if (fileSize <= 0)
	{
		fclose(file);
		return false;
	}

	// 할당 크기와 읽기 크기를 size_t로 통일.
	const size_t bufferSize = static_cast<size_t>(fileSize);
	char* buffer = new char[bufferSize] {};
	// readSize - 실제로 읽은 크기.
	size_t readSize = fread(buffer, sizeof(char), bufferSize, file);
	// readFailed - 읽기 오류가 있었는지 기록.
	bool readFailed = ferror(file) != 0;

	if (readFailed || readSize > bufferSize)
	{
		delete[] buffer;
		fclose(file);
		return false;
	}

	MapData loadedData;
	int x = 0;
	int y = 0;

	for (size_t index = 0;
		index < readSize && index < static_cast<size_t>(fileSize);
		++index)
	{
		// 첫 줄이 끝나면 너비를 기록하고, 줄바꿈마다 x는 0으로 y는 다음 줄로 이동.
		char mapCharacter = buffer[index];

		if (mapCharacter == '\n')
		{
			if (y == 0)
			{
				// 임시로 결과를 모은 뒤, 검증에 성공했을 때만 outData로 넘김.
				loadedData.width = x;
			}
			
			// 빈 줄이나 너비가 다른 줄은 실패 처리.
			// 중간에 돌아가므로 버퍼와 파일 역시 정리.
			if (x == 0 || x != loadedData.width)
			{
				delete[] buffer;
				fclose(file);
				return false;
			}
			x = 0;
			++y;
			continue;
		}

		// 기본 지형은 바닥.
		// #·e·d를 만나면 해당 지형으로 바꿔서 저장.
		// 플레이어·좀비·회복약 자리도 지형은 바닥.

		TileType tile = TileType::Floor;

		switch (mapCharacter)
		{
		case '#':
			tile = TileType::Wall;
			break;

		case 'e':
			tile = TileType::Exit;
			break;

		case 'd':
			tile = TileType::ClosedDoor;
			break;

		// 플레이어의 시작 위치 하나를 기록.
		case 'p':
			loadedData.playerStart = Craft::Vector2(x, y);
			break;

		// 여럿일 수 있으니 목록 추가.
		case 'z':
			loadedData.zombieStarts.emplace_back(x, y);
			break;

		// 여럿일 수 있으니 목록 추가.
		case 'm':
			loadedData.medicinePositions.emplace_back(x, y);
			break;

		default:
			break;
		}

		loadedData.tiles.emplace_back(tile);

		++x;
	}

	delete[] buffer;

	fclose(file);

	if (y == 0)
	{
		loadedData.width = x;
	}

	if (loadedData.width <= 0
		|| (x > 0 && x != loadedData.width))
	{
		return false;
	}

	// 마지막 줄에 줄바꿈이 없어도 높이에 포함하고, 완성된 결과를 전달함.
	// y는 읽은 줄 수가 아니라 줄바꿈을 만난 횟수이기 때문.
	loadedData.height = y + (x > 0 ? 1 : 0);
	outData = std::move(loadedData);
	return true;
}