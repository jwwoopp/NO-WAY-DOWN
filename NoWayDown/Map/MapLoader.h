#pragma once

#include <Map/MapData.h>
#include <string>

class MapLoader
{
public:

	// Load는 파일을 읽어 outData를 채우고, 성공하면 true 반환.
	// static이므로 MapLoader 객첼르 따로 만들지 않고, MapLoader::Load로 호출할 수 있음.
	static bool Load(const std::string& filename, MapData& outData);
};