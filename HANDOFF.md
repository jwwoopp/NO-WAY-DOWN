# NO WAY DOWN 개발 인수인계

## 프로젝트 정보

- 작업 위치: `C:\Workspace\Monapark\Part2Game`
- 저장소: `https://github.com/jwwoopp/no-way-down.git`
- 현재 브랜치: `01.Engine`
- 개발 환경: Windows, Visual Studio, C++20
- 학습용 엔진: `CraftEngine`
- 기준 강사 저장소: `https://github.com/hamtol2/Wanted5_ConsoleGameProject`
- 구현 순서는 강사 저장소의 `1.EngineFramework → 2.Level-Actor → 3.Input` 브랜치를 우선 따른다.

## 게임 MVP

게임 이름은 **NO WAY DOWN**이다.
플레이어가 좀비를 피해 단층 2D 격자 맵을 통과하고 계속 위층으로 올라가는 수직 생존 로그라이트다.
첫 완성 범위는 `4개 층 + 옥상`, 플레이 시간 5~10분이다.
각 층은 별도의 2D Level이며, 위쪽 출구에 도착하면 다음 Level로 전환한다.
플레이어의 체력과 아이템은 유지하고, 아래층의 소음과 체류 시간이 다음 층 난이도에 영향을 준다.
좀비 경로 탐색은 층 내부 A*를 사용한다. 쿼드트리와 3D 높낮이 구현은 MVP에서 제외한다.

## 개발 단계

1. `01.Engine`: 엔진 루프, Level/Actor, Input, Renderer, 더블 버퍼링
2. `02.Floor`: 타일 맵, 플레이어 이동, 벽/문, 층 전환
3. `03.Zombie`: A*, 시야, 소음, 좀비 상태
4. `04.Combat`: 전투, 아이템, 행동 기반 난이도, 사망/탈출

## 현재 실제 코드 상태

- `Engine`에는 120 FPS 목표의 QPC 기반 반복문과 ESC 종료가 구현되어 있다.
- 구조는 `Engine → Level → Actor` 방향으로 만드는 중이다.
- `Actor.h/.cpp`에는 생성자, 가상 소멸자, BeginPlay, Tick, Draw, Destroy, owner가 있다.
- Actor의 owner는 `weak_ptr<Level>`이며, GetOwner에서 lock하여 `shared_ptr`로 반환한다.
- 현재 저장된 `Actor.h`에는 `HasBeganPlay`, `IsActive`, `HasExpired`가 없다.
- `Level.h`에는 생명주기 함수 선언, Actor 벡터 2개, SpawnActor 템플릿이 있다.
- 현재 저장된 `Level.cpp`에는 `#include "Level.h"`만 있고 함수 구현은 아직 없다.
- 따라서 다음 작업은 강사 브랜치 2의 코드를 정확히 확인한 뒤 Level.cpp와 필요한 Actor 조회 함수를 완성하는 것이다.
- 이후 Engine이 Level을 소유하고 생명주기를 호출하도록 연결한 다음 branch 3 Input으로 이동한다.

## Git 주의사항

현재 작업 트리는 커밋 전 변경사항이 있다.

- 삭제 표시: `CraftEngine/Actor.cpp` (중복 파일을 제거한 것)
- 수정: `CraftEngine/Actor/Actor.cpp`, `Actor.h`, 프로젝트 설정 파일, `Engine.cpp`, `Level.h`
- 기존 변경을 되돌리거나 덮어쓰지 말고 먼저 `git diff`와 빌드 상태를 확인한다.
- 커밋 메시지는 한국어로 작성한다.
- 예: `기능: 액터와 레벨 생명주기 구현`

## 사용자와 진행하는 방식

- 사용자가 코드를 직접 입력한다. 완성 코드를 한꺼번에 대신 작성하지 않는다.
- 한 번에 한 줄 또는 함수 하나 정도만 제시하고, 사용자가 입력하면 다음으로 넘어간다.
- 이미 배운 개념은 반복 설명하지 않는다.
- 질문은 설계상 중요한 지점에서만 한다.
- 새 줄을 설명할 때 C++ 문법, 메모리/소유권, 운영체제, 엔진 구조와 연결하되 쉽게 말한다.
- 설명 본문은 보통 10줄, 최대 15줄로 제한한다. 요약은 길어도 된다.
- 실제 코드는 반드시 코드 블록으로 제공한다.
- 강사 저장소에 없는 내용을 강사 코드라고 단정하지 않는다.
- `Core/Core.h`와 DLL 관련 구조는 강사 branch 4부터이므로 branch 2/3 단계에 미리 섞지 않는다.
- 사용자가 오류를 보내면 원인 하나씩 확인하고, 수정 후 빌드를 요청한다.

## 면접용 MVP 설명

Engine은 게임 반복문을 실행하고, Level은 현재 맵과 Actor들을 관리한다.
Actor는 플레이어와 좀비 같은 게임 객체의 부모 클래스다.
Level은 `shared_ptr`로 Actor를 소유하고 Actor는 `weak_ptr`로 Level을 바라봐 순환 소유를 막는다.
Actor는 순회 중 즉시 제거하지 않고 삭제 표시 후 안전한 시점에 제거한다.

## 바로 이어서 할 일

1. `git diff`와 현재 빌드 오류 확인
2. 강사 저장소 `2.Level-Actor`의 Actor/Level 파일과 현재 파일 비교
3. Actor 조회 함수 3개 추가 여부 확인
4. Level.cpp 생명주기와 Actor 추가/삭제 처리 구현
5. 빌드 및 실행 확인
6. Engine과 Level 연결
7. 한국어 커밋 후 `3.Input` 진행
