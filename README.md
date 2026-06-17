# TwinCAT Humanoid Control

휴머노이드 로봇의 **TwinCAT XAE C++ TcCOM 모듈**(실시간 EtherCAT/CiA 402 제어)과 **ADS 클라이언트 GUI**를 한 워크스페이스에서 관리하는 저장소입니다.

- **TwinCAT 측**: 모터 드라이브를 실시간 제어하고, ADS 심볼(`ClientToServer` / `ServerToClient` / `LogBuf`)로 외부 클라이언트와 데이터를 주고받습니다.
- **클라이언트 측**: [`twincat_ads_gui`](https://github.com/hajunsong/twincat_ads_gui) (Git submodule)가 Qt 기반 ADS GUI로 명령·모니터링·로깅을 수행합니다.
- **공유 코드**: `common/` 아래 header-only C++ 라이브러리(CiA 402 상수, 궤적 함수, 모터 수 상수 등)를 각 TwinCAT 모듈이 include 합니다.

원격 저장소: [hajunsong/twincat_humanoid_control](https://github.com/hajunsong/twincat_humanoid_control)

---

## 저장소 구조

```
HumanoidControl/
├── TcLowerBodyControl/      # 하체 15모터 TwinCAT 모듈 (주 개발 대상)
├── TcUpperBodyControlMini/  # 상체 7모터 미니 모듈
├── TcModuleControl/         # 단일 모터 제어 모듈 (공통 명령어 어휘 공유)
├── common/                  # 공유 header-only C++ (TwinCAT 비의존)
├── docs/                    # 인수인계·기술 문서
├── logging/                 # ADS GUI 런타임 CSV 로그 (git 제외)
└── twincat_ads_gui/         # ADS 클라이언트 GUI (Git submodule)
```

| 경로 | 설명 |
|------|------|
| `TcLowerBodyControl/` | 하체 **15모터**. jog·경로·안전 오버라이드 등 최신 제어 로직. 솔루션: `TcLowerBodyControl.slnx` |
| `TcUpperBodyControlMini/` | 상체 **7모터** 미니 버전. 솔루션: `TcUpperBodyControlMini.slnx` |
| `TcModuleControl/` | **단일 모터** TcCOM 모듈. 배열 없이 스칼라 데이터 영역이지만 명령 프로토콜은 동일 |
| `common/humanoid_common/` | 모터 수, CiA 402 상수, 공통 타입 |
| `common/humanoid_motion/` | 5차/사다리꼴 궤적 함수 (`trajectory_profile.h`) |
| `common/humanoid_{kinematics,dynamics,safety}/` | placeholder (향후 확장) |
| `docs/HANDOVER_TwinCAT_ADS.md` | ADS 데이터 영역·명령 프로토콜·사이클 동작 상세 인수인계 문서 |
| `twincat_ads_gui/` | 별도 저장소 submodule — 빌드·실행 방법은 해당 폴더 `README.md` 참고 |

---

## 사전 요구 사항

### TwinCAT 프로젝트

- Beckhoff **TwinCAT XAE** (Visual Studio 통합)
- 타깃 플랫폼: **TwinCAT OS (x64-E)**
- EtherCAT CiA 402 호환 드라이브

### ADS GUI (`twincat_ads_gui`)

- CMake 3.16+, C++17, Qt 5.12+ (`Widgets`, `Network`, `PrintSupport`)
- `third_party/Beckhoff.ADS` (submodule 클론 후 해당 저장소 README 참고)

---

## 저장소 받기

### 처음 clone (submodule 포함, 권장)

```powershell
git clone --recurse-submodules https://github.com/hajunsong/twincat_humanoid_control.git
cd twincat_humanoid_control
```

### 이미 clone한 경우 — submodule 초기화

```powershell
git submodule update --init --recursive
```

### 일상적인 pull (submodule 커밋까지 맞추기)

```powershell
git pull --recurse-submodules
```

또는 pull 후:

```powershell
git submodule update --init --recursive
```

### submodule을 원격 최신 `master`로 올리기

부모 저장소에 기록된 커밋이 아니라 **항상 최신**을 쓰려면:

```powershell
git submodule update --remote twincat_ads_gui
```

이후 submodule 포인터 변경을 부모 저장소에 커밋·push해야 다른 환경과 버전이 맞습니다.

> **참고:** 일반 `git pull`만으로는 submodule 내부가 자동 갱신되지 않습니다. 위 명령을 사용하세요.

---

## TwinCAT 모듈 빌드·배포

1. Visual Studio에서 해당 솔루션(`.slnx`)을 엽니다.  
   - 하체 개발: `TcLowerBodyControl/TcLowerBodyControl.slnx`
2. 구성 **Release**, 플랫폼 **TwinCAT OS (x64-E)** 선택
3. C++ 모듈(`Controller`) 빌드
4. TMC(데이터 영역)를 변경했다면 **TMC Code Generation** 후 `ControllerServices.h` 갱신 확인
5. TwinCAT에서 타깃에 **Activate Configuration**

### 수정 시 주의

| 파일 | 역할 |
|------|------|
| `Module.h` / `Module.cpp` | 수기 제어 로직 — 여기서 개발 |
| `ControllerServices.h`, `Controller.tmc`, `*.vcxproj` | TMC/도구 자동 생성 — **임의 수정 금지** |
| `common/` | 공유 헤더 — TwinCAT 비의존 순수 C++ |

---

## ADS 통신 개요

- **포트**: ADS Port **350** (관측 기준)
- **방식**: 커스텀 IndexGroup/Offset 패킷이 아니라 **ADS 심볼(데이터 영역)** 읽기/쓰기
- **명령**: 클라이언트가 `ClientToServer`에 `MainCmd` / `SubCmd` 등을 쓰면, 모듈이 매 사이클 해석해 EtherCAT 출력에 반영
- **모니터링**: `ServerToClient`, `LogBuf` 읽기

자세한 필드 정의·명령 열거형·사이클 순서는 [`docs/HANDOVER_TwinCAT_ADS.md`](docs/HANDOVER_TwinCAT_ADS.md)를 참고하세요.

### 표준 운용 시퀀스 (요약)

1. `SetOperationMode` (CSP / CSV / CST)
2. `ControlDrive(EnableSequence)` 반복 → Operation Enabled
3. `JogModeMove` 또는 `PathMotion` 명령  
   - Jog는 드라이브가 **CSV(9)** 모드일 때만 동작

---

## ADS GUI (`twincat_ads_gui`)

Submodule로 연결된 Qt 데스크톱 앱입니다.

- **저장소**: [hajunsong/twincat_ads_gui](https://github.com/hajunsong/twincat_ads_gui)
- **빌드·실행·토폴로지 이미지·플롯 스크립트**: [`twincat_ads_gui/README.md`](twincat_ads_gui/README.md)
- **인수인계(클라이언트)**: [`twincat_ads_gui/doc/ads-client-handover.md`](twincat_ads_gui/doc/ads-client-handover.md)

앱이 기록하는 CSV는 워크스페이스 루트 `logging/<세션타임스탬프>/` 아래에 생성됩니다. 이 폴더는 `.gitignore`로 제외됩니다.

---

## `.gitignore` 요약

버전 관리에서 제외하는 주요 항목:

| 구분 | 패턴 |
|------|------|
| TwinCAT 빌드·배포 | `**/_Boot/`, `**/_products/`, `**/_Repository/` |
| Visual Studio | `.vs/`, `*.user`, `Debug/`, `Release/` 등 |
| 런타임 로그 | `logging/`, `*.csv` |
| Scope 기록 | `*.svdx`, `**/.vsm` |
| 백업 | `*.tsproj.bak`, `*.dupfix-bak` |

전체 규칙은 [`.gitignore`](.gitignore)를 참고하세요.

---

## 관련 문서

| 문서 | 내용 |
|------|------|
| [`docs/HANDOVER_TwinCAT_ADS.md`](docs/HANDOVER_TwinCAT_ADS.md) | TwinCAT 서버·ADS 데이터 영역·명령·안전 로직 |
| [`twincat_ads_gui/README.md`](twincat_ads_gui/README.md) | ADS GUI 빌드·실행 |
| [`twincat_ads_gui/doc/ads-client-handover.md`](twincat_ads_gui/doc/ads-client-handover.md) | ADS 클라이언트 인수인계 |

---

## 라이선스

TwinCAT 모듈·`common/` 코드는 이 프로젝트 정책을 따릅니다. `twincat_ads_gui` 및 `third_party/Beckhoff.ADS`는 각 저장소의 라이선스 조건을 따릅니다.
