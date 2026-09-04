#pragma once

#pragma warning(disable: 4251)

#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)

#if ENGINE_BUILD_DLL
#define CRAFT_API DLLEXPORT
#else
#define CRAFT_API DLLIMPORT
#endif