#pragma once

#include <Windows.h>
#include <filesystem>
#include <system_error>

namespace Craft
{
	// The build copies resources beside the architecture directory (Bin/x64/Assets).
	// A packaged game may instead keep them next to the executable.
	inline std::filesystem::path ResolveResourcePath(
		const std::filesystem::path& relativePath)
	{
		wchar_t buffer[32768] = {};
		const DWORD length = GetModuleFileNameW(nullptr, buffer, 32768);
		if (length == 0 || length >= 32768)
		{
			return {};
		}

		const std::filesystem::path executableDirectory =
			std::filesystem::path(buffer, buffer + length).parent_path();
		const std::filesystem::path buildResourcePath =
			executableDirectory.parent_path() / relativePath;

		std::error_code error;
		if (std::filesystem::exists(buildResourcePath, error))
		{
			return buildResourcePath;
		}

		return executableDirectory / relativePath;
	}
}
