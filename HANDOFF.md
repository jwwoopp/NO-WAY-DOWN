# NO WAY DOWN 개발 인수인계

최종 갱신: 2026-09-03

## 프로젝트 정보

- 작업 위치: `C:\Workspace\Monapark\Part2Game`
- 저장소: `https://github.com/jwwoopp/no-way-down.git`
- 현재 브랜치: `01.Engine` (원격과 동기화됨)
- 개발 환경: Windows, Visual Studio 18, C++20, x64 Debug
- 기준 강사 저장소: `https://github.com/hamtol2/Wanted5_ConsoleGameProject`
- 구현 순서는 강사 저장소 브랜치를 따른다. 현재 `4.EngineDLL-Seperation`까지 완료.

## 게임 MVP

게임 이름은 **NO WAY DOWN**이다.
플레이어가 좀비를 피해 단층 2D 격자 맵을 통과하고 계속 위층으로 올라가는 수직 생존 로그라이트다.
첫 완성 범위는 `4개 층 + 옥상`, 플레이 시간 5~10분이다.
각 층은 별도의 2D Level이며, 위쪽 출구에 도착하면 다음 Level로 전환한다.
플레이어의 체력과 아이템은 유지하고, 아래층의 소음과 체류 시간이 다음 층 난이도에 영향을 준다.
좀비 경로 탐색은 층 내부 A*를 사용한다. 쿼드트리와 3D 높낮이 구현은 MVP에서 제외한다.

## 개발 단계

1. `01.Engine`: 엔진 루프, Level/Actor, Input, Renderer, 더블 버퍼링  ← 진행 중
2. `02.Floor`: 타일 맵, 플레이어 이동, 벽/문, 층 전환
3. `03.Zombie`: A*, 시야, 소음, 좀비 상태
4. `04.Combat`: 전투, 아이템, 행동 기반 난이도, 사망/탈출

## 2026-09-03 작업 내역

커밋 5개. 강사 브랜치 2~4에 해당하는 범위를 완료했다.

| 커밋 | 내용 | 강사 브랜치 |
| --- | --- | --- |
| `0a677d8` | 액터와 레벨 생명주기 구현 및 엔진 연결 | 2.Level-Actor |
| `0454eb4` | 테스트 레벨과 액터 추가로 엔진 연결 확인 | 2.Level-Actor |
| `a8be092` | 입력 시스템 추가 | 3.Input |
| `c79cd78` | 엔진을 DLL로 분리하고 CRAFT_API 적용 | 4.EngineDLL-Seperation |
| `9d7a5d3` | 엔진 DLL 분리 및 게임 프로젝트 NoWayDown 구성 | 4.EngineDLL-Seperation |

## 현재 프로젝트 구조

솔루션 `Part2Game.slnx`에 프로젝트 2개.

```
CraftEngine (DynamicLibrary → CraftEngine.dll)     엔진
  Core/Core.h            CRAFT_API 매크로 (dllexport / dllimport 전환)
  Engine/Engine.h .cpp   게임 루프, Level·Input 소유
  Level/Level.h  .cpp    액터 목록과 생명주기
  Actor/Actor.h  .cpp    게임 객체 기반 클래스
  Input/Input.h  .cpp    키보드 상태 관리

NoWayDown (Application → NoWayDown.exe)            게임
  Actor/TestActor.h .cpp
  Level/TestLevel.h .cpp
  Main.cpp
```

`WantedPart2` 프로젝트는 솔루션과 git에서 제거했다.

## 현재 코드 상태

전부 빌드·실행 확인됨. `Bin\x64\Debug\`에 `CraftEngine.dll`과 `NoWayDown.exe`가 나온다.

**Engine**
- QPC 기반 120 FPS 고정 프레임 루프.
- 프레임 순서: `ProcessInput()` → ESC 검사 → `OnInitialized` → `BeginPlay` → `Tick` → `Draw` → 레벨 교체 → `ProcessAddAndDestroyActors` → `SavePreviousInputStates()`.
- `mainLevel`(현재 층)과 `nextLevel`(예약된 층)을 `shared_ptr`로 소유. 프레임이 끝난 뒤에만 교체한다.
- `AddNewLevel<T>()` 템플릿으로 다음 레벨 예약.
- `input`을 `unique_ptr`로 소유.

**Level**
- `OnInitialized` / `BeginPlay` / `Tick` / `Draw` / `ProcessAddAndDestroyActors` 구현 완료.
- `SpawnActor<T>()`는 추가 예약 목록에 넣고, 프레임 끝에 `actorList`로 옮긴다.
- 액터 제거도 순회 중이 아니라 프레임 끝에 처리한다.

**Actor**
- `HasBeganPlay()` / `IsActive()` / `HasExpired()` 구현. `IsActive()`는 `isActive && !hasExpired`.
- `Destroy()`는 즉시 삭제가 아니라 `hasExpired` 플래그만 켠다.
- owner는 `weak_ptr<Level>`, `GetOwner()`에서 lock 해서 반환.

**Input**
- 키마다 `isKeyDown`(지금) / `wasKeyDown`(이전 프레임) 두 값을 저장한다.
- `GetKeyDown` / `GetKeyUp` / `GetKey` 세 가지 판정 제공.
- `ProcessInput()`과 `SavePreviousStates()`는 `private`이고 `friend class Engine`으로 Engine만 호출한다.
- 전역 접근은 `Input::Get()`.

**TestActor / TestLevel**
- `TestLevel::OnInitialized()`에서 `SpawnActor<TestActor>()`.
- `TestActor::Tick()`은 A 키 입력 3종을 콘솔에 출력한다. Renderer 단계에서 방향키 이동으로 교체 예정.

## 강사 코드와 다른 점

의도적으로 뺐거나 아직 안 넣은 것들. 다음 단계에서 필요해지면 그때 넣는다.

- `Engine::Get()` 싱글톤과 `static Engine* instance`가 없다. 따라서 `Actor::QuitGame()`도 없다.
  ESC 종료는 `Engine::Run()` 안에서 `input->GetKeyDown(VK_ESCAPE)`로 직접 처리한다.
  강사 코드는 `TestActor::Tick()`에서 `QuitGame()`을 부른다.
- `Level::FindActor<T>()` 템플릿이 없다.
- `Engine::Shutdown()`이 없다.
- 게임 프로젝트 이름이 `Game`이 아니라 `NoWayDown`이다.

## 프로젝트 설정 (오늘 시간을 많이 쓴 부분)

**CraftEngine**
- 구성 형식: `동적 라이브러리(.dll)`
- 전처리기 정의에 `ENGINE_BUILD_DLL` (이 이름표로 `Core.h`가 export/import를 가른다)
- 출력 디렉터리: `$(SolutionDir)Bin\$(Platform)\$(Configuration)\`
- 중간 디렉터리: `$(SolutionDir)Intermediate\$(Platform)\$(Configuration)\$(ProjectName)\`
- 빌드 전 이벤트: `xcopy *.h ..\Includes\ /e /y /i`
- 빌드 후 이벤트: `xcopy $(OutDir)\*.lib ..\Lib\ /e /y /i`

**NoWayDown**
- 구성 형식: `애플리케이션(.exe)`
- 추가 포함 디렉터리: `..\Includes;$(ProjectDir);%(AdditionalIncludeDirectories)`
- 링커 → 출력 파일: `$(OutDir)$(TargetName)$(TargetExt)`
- 링커 → 추가 라이브러리 디렉터리: `..\Lib;%(AdditionalLibraryDirectories)`
- 링커 → 추가 종속성: `CraftEngine.lib;$(CoreLibraryDependencies);%(AdditionalDependencies)`
- 시작 프로젝트로 설정되어 있다.

`Includes/`, `Lib/`, `Bin/`, `Intermediate/`는 `.gitignore`에 있다.

### 아직 정리 안 된 설정

Debug x64로는 문제없이 돌아가지만 아래는 남아 있다.

- `CraftEngine.vcxproj` 52줄: Release의 중간 디렉터리가 `Bin\...`을 가리킨다. `Intermediate\...`로 고쳐야 한다.
- `CraftEngine.vcxproj` 89줄: Release 전처리기 정의에 `_DEBUG`가 들어가 있다. `NDEBUG`여야 한다.
- `CraftEngine.vcxproj` 61줄: 추가 포함 디렉터리에 `..\Includes\SoundSystem` 잔재가 남아 있다. 이 프로젝트에 사운드 시스템은 없다.
- `NoWayDown.vcxproj` 48줄: Debug 중간 디렉터리에만 `$(ProjectName)`이 빠져 있다. Release(52줄)와 다르다.

## 다음 할 일 — 강사 브랜치 `5.Renderer`

여기서 처음으로 화면에 그림이 나온다. 새 파일 5개, 기존 파일 3개 수정.

**새로 만들 것**

| 파일 | 내용 |
| --- | --- |
| `CraftEngine/Math/Vector2.h .cpp` | 좌표 클래스. 사칙·비교·대입 연산자 오버로딩, `operator COORD()`로 콘솔 좌표 자동 변환, `static Zero/One/Right/Up`. 콘솔은 y가 아래로 증가하므로 `Up`은 `(0, -1)`. |
| `CraftEngine/Math/Color.h` | `enum class Color : WORD`. Windows `FOREGROUND_*` 상수 조합. |
| `CraftEngine/Render/Renderer.h .cpp` | 그리기 전담. `Submit()`으로 렌더 명령을 큐에 쌓고, `Draw()`에서 `Clear` → `DrawRenderQueue` → `Present`. `Renderer::Get()` 싱글톤. 생성자에서 콘솔 커서를 숨기고 소멸자에서 되돌린다. `Clear()`는 임시로 `system("cls")`, `Present()`는 더블 버퍼링 단계에서 채운다. |

**고칠 것**

- `Engine`: `class Renderer;` 전방 선언, `unique_ptr<Renderer> renderer` 멤버, 생성자에서 생성, `Engine::Draw()` 끝에서 `renderer->Draw()` 호출.
- `Actor`: 생성자를 `Actor(const std::string& image = "", const Vector2& position = Vector2::Zero, Color color = Color::White)`로 바꾸고 `image` / `position` / `color` / `width` / `sortingOrder` 멤버 추가. `Draw()`에서 `Renderer::Get().Submit(...)` 호출. `GetPosition()` / `SetPosition()` 추가.
- `TestActor`: 생성자에서 `Actor("P", Vector2(5, 5), Color::Green)`, `sortingOrder = 5`. `Tick()`을 방향키 이동으로 교체 (`VK_LEFT`/`RIGHT`/`UP`/`DOWN`, 화면 경계 0~39 / 0~24).

**주의**: 강사 `TestActor::Tick()`은 ESC에서 `QuitGame()`을 부르는데 우리에겐 그 함수가 없다. ESC는 `Engine::Run()`에서 이미 처리하므로 그 부분은 빼고 진행한다.

`Math`와 `Render` 폴더를 새로 만들면 `xcopy *.h ..\Includes\`가 하위 폴더까지 자동으로 복사한다.

## 그 이후

- `6.EngineSetting`: 설정 파일 읽기. `..\Config\` 폴더가 필요해진다.
- `7.DoubleBuffering`: `Renderer::Present()`를 채워 깜빡임 제거. 여기까지가 `01.Engine` 단계의 끝.
- `01.Engine`을 닫으면 `main`에 머지하고 거기서 `02.Floor`를 딴다.

## 사용자와 진행하는 방식

- 사용자가 코드를 직접 입력한다. 완성 코드를 한꺼번에 대신 작성하지 않는다.
- 한 번에 한 줄 또는 함수 하나 정도만 제시하고, 사용자가 입력하면 다음으로 넘어간다.
- 설명은 **아주 쉽게** 한다. 비유를 쓰고 문장을 짧게 끊는다. 사용자가 "더 쉽게"라고 하면 용어를 더 덜어낸다.
- 이미 배운 개념은 반복 설명하지 않는다.
- 질문은 설계상 중요한 지점에서만 한다.
- 실제 코드는 반드시 코드 블록으로 제공한다.
- 생략 표시로 `...`을 코드 안에 쓰지 않는다. 그대로 입력해서 빌드가 깨진 적이 있다.
- 강사 저장소에 없는 내용을 강사 코드라고 단정하지 않는다. 확인이 필요하면 저장소를 직접 열어본다.
- 사용자가 오류를 보내면 원인 하나씩 확인하고, 수정 후 빌드를 요청한다.

## 자주 반복된 실수 (확인용 체크리스트)

- **저장 안 함**: VS에서 편집만 하고 `Ctrl+S`를 안 눌러 파일에 반영이 안 된 경우가 여러 번 있었다. 프로젝트 속성은 `적용`을 눌러도 `Ctrl+Shift+S`가 필요할 때가 있다.
- **중복 붙여넣기**: 직접 타이핑한 코드 위에 같은 코드를 또 붙여넣어 함수가 두 번 정의된 적이 있다 (`Actor.h`의 getter, `Engine.cpp`의 생성자).
- **대소문자**: `KeyCode`/`keyCode`, `KeyStates`/`keyStates`, `Input`/`input`. C++은 구분한다. 타입은 대문자 시작, 변수는 소문자 시작.
- **중괄호 개수**: `Run()`을 닫는 `}`를 빠뜨려 뒤 함수들이 전부 이상한 에러를 냈다. 에러가 여러 개면 맨 위 것부터 본다.
- **설정 칸 헷갈림**: 포함 디렉터리 값을 링커 출력 파일 칸에 넣은 적이 있다. 칸을 비우면 VS가 빈 값을 써서 링커가 엉뚱한 이름의 exe를 만든다.
- **터미널**: 사용자는 cmd를 쓴다. `rm`, `ls`, `cat`이 없다. `rmdir /s /q`, `dir`, `type`을 쓴다.

## 면접용 MVP 설명

Engine은 게임 반복문을 실행하고, Level은 현재 맵과 Actor들을 관리한다.
Actor는 플레이어와 좀비 같은 게임 객체의 부모 클래스다.
Level은 `shared_ptr`로 Actor를 소유하고 Actor는 `weak_ptr`로 Level을 바라봐 순환 소유를 막는다.
Actor는 순회 중 즉시 제거하지 않고 삭제 표시 후 안전한 시점에 제거한다.
엔진은 DLL로 분리되어 있고, 게임 프로젝트는 헤더와 lib만 가져다 쓴다.
같은 헤더를 엔진이 읽으면 dllexport, 게임이 읽으면 dllimport가 되도록 `CRAFT_API` 매크로로 전환한다.
