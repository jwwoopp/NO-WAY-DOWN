# NO WAY DOWN 개발 인수인계

최종 갱신: 2026-09-04

## 기본 정보

- 작업 폴더: `C:\Workspace\Monapark\Part2Game`
- 저장소: `https://github.com/jwwoopp/no-way-down.git`
- 현재 브랜치: `01.Engine`
- 개발 환경: Windows, Visual Studio 18, C++20, x64 Debug
- 기준 강사 저장소: `https://github.com/hamtol2/Wanted5_ConsoleGameProject`
- 강사 브랜치 순서대로 진행 중. **`7.DoubleBuffering`까지 완료.**

## 사용자와 진행하는 방식

- 사용자가 코드를 직접 입력한다. 대신 파일을 완성해주지 않는다.
- 한 번에 한 줄 또는 함수 하나 이하로 나누어 안내한다.
- 수정이나 추가 위치는 반드시 **현재 파일명과 줄 번호**로 말한다.
- 줄 번호는 매번 실제 파일을 다시 읽고 확인한다.
- 코드는 반드시 코드 블록으로 제공하고, 코드 안에 `...` 생략 표시를 쓰지 않는다. 그대로 입력해서 빌드가 깨진 적이 있다.
- 설명은 **아주 쉽게** 한다. 비유를 쓰고 문장을 짧게 끊는다. 사용자가 "더 쉽게"라고 하면 용어를 더 덜어낸다.
- 이미 이해한 개념은 반복 설명하지 않는다.
- 질문은 설계상 중요한 것만 한다.
- 사용자가 오류를 보내면 **가장 첫 오류부터** 원인 하나씩 해결한다.
- 강사 코드를 확인하지 않고 추측해서 안내하지 않는다. 저장소를 직접 열어본다.
- 강사 코드에 없는 것을 제안할 때는 **그렇다고 미리 밝힌다.**
- 사용자는 한글 주석을 많이 쓴다. C4819가 나오면 해당 파일을 **UTF-8 서명 포함**으로 저장한다.
- 폴더 이름은 강사의 `Render`와 다르게 사용자가 `Renderer`로 정했다. include 경로는 `<Renderer/Renderer.h>`로 통일한다.
- 커밋 메시지는 한국어로 작성한다.
- 사용자 터미널은 **cmd**다. `rm`, `ls`, `cat`이 없다. `rmdir /s /q`, `del`, `dir`, `type`을 쓴다.

## 게임 MVP 방향

- 게임명: **NO WAY DOWN**
- 탑다운 문자/블록 기반 수직 좀비 생존 로그라이트
- 한 층은 독립적인 2D 격자 `Level`
- 한 층 탐색 → 위쪽 출구 도달 → 다음 `Level` 진입이 최소 게임 루프
- 첫 목표는 `4개 층 + 옥상`, 5~10분 데모
- 층 내부 좀비 길찾기는 A* 사용
- 층 전환은 A*가 아니라 Level 교체로 처리
- 쿼드트리와 3D 높낮이는 MVP에서 제외
- RunState는 층 전환이 작동한 뒤 체력 하나부터 추가
- 16개 콘솔 색상 슬롯의 실제 RGB를 0~255로 바꾸는 커스텀 팔레트를 Renderer에 추가할 예정

## 개발 단계

1. `01.Engine`: 엔진 루프, Level/Actor, Input, Renderer, 더블 버퍼링  ← 거의 완료
2. `02.Floor`: 타일 맵, 플레이어 이동, 벽/문, 층 전환
3. `03.Zombie`: A*, 시야, 소음, 좀비 상태
4. `04.Combat`: 전투, 아이템, 행동 기반 난이도, 사망/탈출

## 진행 내역

| 커밋 | 내용 | 강사 브랜치 |
| --- | --- | --- |
| `0a677d8` | 액터와 레벨 생명주기 구현 및 엔진 연결 | 2.Level-Actor |
| `0454eb4` | 테스트 레벨과 액터 추가로 엔진 연결 확인 | 2.Level-Actor |
| `a8be092` | 입력 시스템 추가 | 3.Input |
| `c79cd78` | 엔진을 DLL로 분리하고 CRAFT_API 적용 | 4.EngineDLL-Seperation |
| `9d7a5d3` | 엔진 DLL 분리 및 게임 프로젝트 NoWayDown 구성 | 4.EngineDLL-Seperation |
| `15c0bad` | 프로젝트 빌드 경로 정리 | — |
| `c72a912` | 콘솔 렌더러와 액터 이동 구현 | 5.Renderer |
| (미커밋) | 엔진 설정 파일 로드와 이중 버퍼링 구현 | 6.EngineSetting, 7.DoubleBuffering |

## 프로젝트 구조

솔루션 `Part2Game.slnx`에 프로젝트 2개.

```
CraftEngine (DynamicLibrary → CraftEngine.dll)
  Core/Core.h                    CRAFT_API 매크로 (dllexport / dllimport 전환)
  Engine/Engine.h .cpp           게임 루프, 설정 로드, Input·Renderer 소유
  Level/Level.h  .cpp            액터 목록과 생명주기
  Actor/Actor.h  .cpp            게임 객체 기반 클래스
  Input/Input.h  .cpp            키보드 상태 관리
  Math/Vector2.h .cpp            좌표, COORD 변환, 연산자 오버로딩
  Math/Color.h                   콘솔 색상 enum
  Renderer/Renderer.h .cpp       렌더 큐, Frame 배열, 이중 버퍼 관리
  Renderer/ScreenBuffer.h .cpp   콘솔 화면 한 장 (Windows 핸들)

NoWayDown (Application → NoWayDown.exe)
  Actor/TestActor.h .cpp
  Level/TestLevel.h .cpp
  Main.cpp

Config/Setting.txt               framerate / width / height
```

`Includes/`, `Lib/`, `Bin/`, `Intermediate/`는 `.gitignore`에 있다.

## 현재 코드 상태

전부 빌드·실행 확인됨. 초록색 `P`가 방향키로 움직이고 ESC로 종료된다. 깜빡임 없음.

**Engine**
- QPC 기반 고정 프레임 루프. 목표 프레임은 `Config/Setting.txt`에서 읽는다.
- 프레임 순서: `ProcessInput()` → ESC 검사 → `OnInitialized` → `BeginPlay` → `Tick` → `Draw` → 레벨 교체 → `ProcessAddAndDestroyActors` → `SavePreviousInputStates()`.
- 생성자 순서가 중요하다. `LoadEngineSetting()` → `Input` 생성 → `Renderer` 생성(화면 크기 전달).
- 엔진은 ESC를 모른다. 종료는 `TestActor::Tick()`에서 `Input::Get().GetKeyDown(VK_ESCAPE)` → `QuitGame()` → `Engine::Get().Quit()`. 엔진은 수단만 주고 판단은 게임이 한다.
- `Engine::Get()` 싱글톤 있음. `GetWidth()` / `GetHeight()` 있으나 아직 아무도 안 쓴다 (강사도 브랜치 19에서야 사용).
- `LoadEngineSetting()`은 `../Config/Setting.txt`를 `fopen_s`로 열고, 2048바이트 버퍼에 `fread`, `strtok_s`로 줄 단위 분리, `sscanf_s`로 값 파싱.

**Renderer**
- `Submit()`으로 렌더 명령을 큐에 쌓고, `Draw()`에서 `Clear` → `DrawRenderQueue` → `Present`.
- `Frame`은 화면 칸 수만큼의 `CHAR_INFO[]`와 `int[]`(sortingOrder) 배열. 1차원 배열을 `(y * width) + x`로 2차원처럼 쓴다.
- `DrawRenderQueue`는 화면 밖 판정 → 좌우 클리핑 → sortingOrder 비교 후 배열에 기록. 루프가 끝난 뒤 `WriteConsoleOutputA`로 **한 번만** 출력한다.
- `ScreenBuffer` 2개를 만들어 `SetConsoleActiveScreenBuffer`로 번갈아 보여준다. 인덱스는 `1 - currentBufferIndex`로 뒤집는다.
- `system("cls")`는 제거됐다.

**Actor**
- `image` / `position` / `color` / `width` / `sortingOrder` 보유.
- `Draw()`에서 `Renderer::Get().Submit(...)`.
- `Destroy()`는 즉시 삭제가 아니라 `hasExpired` 플래그만 켠다. 실제 제거는 프레임 끝.
- owner는 `weak_ptr<Level>`.

**Input**
- 키마다 `isKeyDown`(지금) / `wasKeyDown`(이전 프레임) 두 값 저장.
- `GetKeyDown` / `GetKeyUp` / `GetKey`.
- 갱신 함수는 `private` + `friend class Engine`.

**Level**
- 액터 추가와 제거 모두 프레임 끝에 `ProcessAddAndDestroyActors()`에서 처리한다.

## 강사 코드와 다른 점

- 폴더 이름이 `Render`가 아니라 **`Renderer`**다.
- 게임 프로젝트 이름이 `Game`이 아니라 **`NoWayDown`**이다.
- `Level::FindActor<T>()` 템플릿이 없다.
- `Engine::Shutdown()`이 없다.
- `Actor.h` 인자 이름에 오타가 있다 (`Color colr`). 선언부라 빌드는 되지만 헷갈린다.
- `Actor::Draw()`에 강사의 `if (!IsActive()) return;` 검사가 없다. `Level::Draw()`가 이미 걸러서 동작에는 문제없다.

## 프로젝트 설정

**CraftEngine**
- 구성 형식: `동적 라이브러리(.dll)`
- 전처리기 정의에 `ENGINE_BUILD_DLL` (Debug만 들어가 있음 — Release 미설정)
- 출력 디렉터리: `$(SolutionDir)Bin\$(Platform)\$(Configuration)\`
- 중간 디렉터리: `$(SolutionDir)Intermediate\$(Platform)\$(Configuration)\$(ProjectName)\`
- 빌드 전 이벤트: `xcopy *.h ..\Includes\ /e /y /i`
- 빌드 후 이벤트: `xcopy $(OutDir)\*.lib ..\Lib\ /e /y /i`

**NoWayDown**
- 구성 형식: `애플리케이션(.exe)`, 시작 프로젝트
- 추가 포함 디렉터리: `..\Includes;$(ProjectDir);%(AdditionalIncludeDirectories)`
- 링커 → 출력 파일: `$(OutDir)$(TargetName)$(TargetExt)`
- 링커 → 추가 라이브러리 디렉터리: `..\Lib;%(AdditionalLibraryDirectories)`
- 링커 → 추가 종속성: `CraftEngine.lib;$(CoreLibraryDependencies);%(AdditionalDependencies)`

### 정리가 필요한 설정

Debug x64로는 돌아가지만 아래는 어긋나 있다.

- **`Config` 복사 이벤트가 Release 구성에만 있다.** Debug에는 없어서 `Bin\x64\Debug\` 옆에 `Config`가 안 생긴다. 그리고 그 과정에서 Release의 헤더 복사 줄이 사라졌다.
  두 구성 모두 빌드 전 이벤트가 아래 두 줄이어야 한다.
  ```
  xcopy *.h ..\Includes\ /e /y /i
  xcopy ..\Config\* $(OutDir)..\Config\ /e /y /i
  ```
- `CraftEngine.vcxproj` Debug 블록에 헤더 복사 명령이 한 번 더 들어가 있다 (빌드 전 이벤트가 아닌 다른 이벤트). 중복이라 지워도 된다.
- `CraftEngine.vcxproj` Release의 중간 디렉터리가 `Bin\...`을 가리킨다. `Intermediate\...`여야 한다.
- `CraftEngine.vcxproj` Release 전처리기 정의에 `_DEBUG`가 들어가 있고 `ENGINE_BUILD_DLL`이 없다. `NDEBUG;ENGINE_BUILD_DLL`이어야 한다.
- `CraftEngine.vcxproj` Debug 추가 포함 디렉터리에 `..\Includes\SoundSystem` 잔재가 있다. 이 프로젝트에 사운드 시스템은 없다.
- `NoWayDown.vcxproj` Debug 중간 디렉터리에만 `$(ProjectName)`이 빠져 있다.

**실행 경로 주의**: `LoadEngineSetting()`이 `../Config/Setting.txt`를 연다. VS에서 F5로 실행하면 작업 폴더가 `NoWayDown\`이라 맞아떨어진다. `Bin\x64\Debug\NoWayDown.exe`를 직접 실행하려면 위 `Config` 복사 이벤트가 Debug에도 있어야 한다.

## 다음 할 일

**1. 커밋** (아직 안 됨)

```
git add Config CraftEngine NoWayDown
git commit -m "기능: 엔진 설정 파일 로드와 이중 버퍼링 구현"
git push origin 01.Engine
```

**2. `01.Engine` 마무리**

- 위 "정리가 필요한 설정" 처리
- 커스텀 16색 RGB 팔레트 (선택). 콘솔 색상 슬롯의 실제 RGB를 바꾸는 기능
- 최종 빌드·실행 확인 후 `main`에 머지

**3. `02.Floor` 브랜치**

`main`에서 새로 딴다. 여기서부터 실제 게임이다.

- 단일 층 격자 맵 (벽, 바닥, 출구)
- 맵을 `Actor`로 그릴지 `Level`이 직접 그릴지 결정 필요
- 플레이어 이동에 벽 충돌 판정
- 출구 도달 시 `AddNewLevel<T>()`로 다음 층 전환
- `RunState`로 체력 하나 유지 테스트
- `(y * width) + x` 인덱스 변환이 타일 맵에서 계속 쓰인다

## 알아둘 것

- `TestActor`의 경계값 `39`, `24`는 하드코딩이다. 강사도 브랜치 19까지 그대로 뒀다. 우리 게임 코드를 만들 때 `Engine::Get().GetWidth()`로 바꾸면 된다.
- `sortingOrder`는 `7.DoubleBuffering`부터 실제로 겹침 우선순위에 쓰인다. 빈 칸 표시는 `-1`이다.
- `Includes/`는 CraftEngine 공개 헤더 전용이다. `NoWayDown`의 게임 헤더가 여기 들어가면 안 된다. 과거에 `Includes/Actor/TestActor.h`가 남아서 중복 정의가 난 적이 있다.
- `Vector2::operator COORD()`가 Windows 함수 호출을 짧게 해준다. `SetConsoleScreenBufferSize`, `FillConsoleOutputCharacterA`, `WriteConsoleOutputA` 등에서 `Vector2`를 그대로 넘긴다.

## 자주 반복된 실수 (확인용)

- **저장 안 함**: VS에서 편집만 하고 `Ctrl+S`를 안 눌러 파일에 반영이 안 된 경우가 여러 번 있었다. 프로젝트 속성은 `적용`을 눌러도 `Ctrl+Shift+S`가 필요할 때가 있다.
- **중복 붙여넣기**: 직접 타이핑한 코드 위에 같은 코드를 또 붙여넣어 함수가 두 번 정의된 적이 있다.
- **대소문자**: `KeyCode`/`keyCode`, `Input`/`input`. 타입은 대문자 시작, 변수는 소문자 시작.
- **중괄호 개수**: 함수를 닫는 `}`를 빠뜨리면 그 뒤 함수들이 전부 엉뚱한 에러를 낸다. 에러가 여러 개면 맨 위 것부터 본다.
- **설정 칸 헷갈림**: 포함 디렉터리 값을 링커 출력 파일 칸에 넣은 적이 있다. 칸을 비우면 VS가 빈 값을 써서 링커가 엉뚱한 이름의 exe를 만든다.
- **한 구성만 수정**: 프로젝트 속성은 `모든 구성`으로 놓고 고쳐야 Debug/Release 둘 다 적용된다. 값이 서로 다른 항목은 빈칸으로 보이니 구성을 하나씩 바꿔가며 넣는다.

## 면접용 MVP 설명

Engine은 게임 반복문을 실행하고, Level은 현재 맵과 Actor들을 관리한다.
Actor는 플레이어와 좀비 같은 게임 객체의 부모 클래스다.
Level은 `shared_ptr`로 Actor를 소유하고 Actor는 `weak_ptr`로 Level을 바라봐 순환 소유를 막는다.
Actor는 순회 중 즉시 제거하지 않고 삭제 표시 후 안전한 시점에 제거한다.
엔진은 DLL로 분리되어 있고, 게임 프로젝트는 헤더와 lib만 가져다 쓴다.
같은 헤더를 엔진이 읽으면 dllexport, 게임이 읽으면 dllimport가 되도록 `CRAFT_API` 매크로로 전환한다.
렌더링은 액터가 직접 그리지 않고 렌더 명령을 큐에 제출하는 방식이다.
한 프레임의 모든 명령을 메모리 배열에 모은 뒤 콘솔에 한 번만 출력하고, 화면 버퍼 두 개를 번갈아 활성화해 깜빡임을 없앴다.
