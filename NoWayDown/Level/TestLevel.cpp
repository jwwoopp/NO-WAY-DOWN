#include "TestLevel.h"
#include <Actor/TestActor.h>

void TestLevel::OnInitialized()
{
	Craft::Level::OnInitialized();

	SpawnActor<TestActor>();
}