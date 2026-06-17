# 휴머노이드 제어 — TwinCAT / ADS 서버 인수인계 문서

> 최종 정리일: 2026-06-11
> 대상 활성 프로젝트: **TcLowerBodyControl** (하체 15모터)

## 1. 시스템 개요
- TwinCAT XAE **C++ TcCOM 모듈**이 EtherCAT 드라이브(CiA 402)를 실시간 제어하고, 외부 **ADS 클라이언트**가 ADS 심볼(데이터 영역)을 읽고/써서 명령·모니터링한다.
- 통신은 **데이터 영역(심볼) 기반**이다. 커스텀 ADS 패킷 프로토콜(IndexGroup/Offset 직접 파싱)은 **사용하지 않는다.** (`AdsReadWriteInd`/`AdsReadCon`은 자동생성 stub 그대로)
- 모터 명령 모델: 클라이언트가 `ClientToServer`에 명령을 쓰면, 모듈이 매 사이클 해석해 `MotorCmdArea`(EtherCAT 출력)에 반영하고, 상태는 `MotorStArea`(EtherCAT 입력)에서 받아 `ServerToClient`로 미러링한다.

## 2. 솔루션 / 프로젝트 구성 (중요 — 프로젝트가 여러 개)
워크스페이스 루트: `c:\Users\keti-human\Desktop\HumanoidControl`

| 프로젝트 | 솔루션 | 모터 수 | 비고 |
|---|---|---|---|
| **TcLowerBodyControl** | `TcLowerBodyControl.slnx` | 15 | **현재 활성/최신 개발 대상.** 하체. 로깅 영역명이 `LogBuf` |
| TcLowerBodyController | `TcLowerBodyController.sln` | 15 | 구버전 하체. 로깅 영역명이 `LoggingBuffer`(`.Data` 래퍼) |
| TcUpperBodyController | `TcUpperBodyController.sln` | 16 | 상체 |
| TcHumanoidController | `TcHumanoidController.sln` | - | 내부에 TcLowerBodyController 프로젝트 참조 |
| TwinCAT Project1 | `TwinCAT Project1.slnx` | - | 스크래치/임시(정리 대상으로 보임) |

> **인수인계 핵심**: 최근 모든 기능 개발(jog 속도제어, 안전 오버라이드 등)은 **`TcLowerBodyControl`** (이름에 "ler" 없음)에서 진행됨. 나머지 lower 프로젝트(`TcLowerBodyController`)와 혼동 주의. 어떤 프로젝트가 실제 런타임에 배포/Activate되는지 반드시 인계받는 사람에게 명확히 전달할 것.

핵심 소스 위치 (활성 프로젝트):
- `TcLowerBodyControl/TcLowerBodyControl/Controller/Module.h` / `Module.cpp` — 제어 로직(수기 작성)
- `.../Controller/ControllerServices.h` — TMC 자동생성 데이터 타입/ID (수정 금지)
- `.../Controller/Controller.tmc` — 데이터 영역 정의 (TwinCAT에서만 편집)
- `.../Controller/Controller.vcxproj` — include 경로 등

## 3. 공유 헤더 (`common/`, header-only)
`Controller.vcxproj`의 `AdditionalIncludeDirectories`에 등록:
- `..\..\..\common\humanoid_common\include`
- `..\..\..\common\humanoid_motion\include`

| 헤더 | 내용 |
|---|---|
| `humanoid_common/motor_count.h` | `kUpperBodyMotors=16`, `kLowerBodyMotors=15` |
| `humanoid_common/cia402.h` | CiA402 상수: ControlWord(`Shutdown=0x06`, `SwitchOn=0x07`, `EnableOperation=0x0F`, `FaultReset=0x80`), Mode(`CSP=8`,`CSV=9`,`CST=10`), `StateBits()`, `kStatusFaultBit` |
| `humanoid_motion/trajectory_profile.h` | 궤적 함수: `ComputeStepCount`, `Trapezoidal5thAtTime`, `Single5thPosition`, `RoundToLong`, `kTrajectoryMaxSteps=10000` |

> `common/humanoid_kinematics`, `humanoid_dynamics`, `humanoid_safety`는 빈 placeholder(.gitkeep)다.

## 4. ADS 데이터 영역 (TcLowerBodyControl, 15모터)
ADI ID: `MotorStArea=0`, `MotorCmdArea=1`, `ServerToClient=3`, `ClientToServer=4`, `LogBuf=5`
ADS 포트: **350** (심볼 watch의 Port 350), 심볼 경로 prefix `TIXC^Controller^Controller_Obj^...`

```
MotorSt  { USHORT nStatusWord; LONG nActualPosition; LONG nActualVelocity;
           SHORT nActualTorque; USHORT nErrorCode; char nModeOfOperationDisp; bool nWcState; }
MotorCmd { USHORT nControlWord; LONG nTargetPosition; LONG nTargetVelocity;
           SHORT nTargetTorque; USHORT nMaxTorque; char nModeOfOperation; }
PathParameter { double nTotalTime; double nStepSize; double nAccTime;
                USHORT nProfileMode; bool nUpdate; LONG nSetPosition; }
JogParameter  { USHORT nJogID; USHORT nJogTick; }

MotorStArea     { MotorSt  Data[15]; }              // EtherCAT 입력(드라이브 상태)
MotorCmdArea    { MotorCmd Data[15]; }              // EtherCAT 출력(드라이브 명령)
ServerToClient  { MotorSt DataMotorSt[15]; MotorCmd DataMotorCmd[15]; }   // 모니터링용 미러
ClientToServer  { USHORT MainCmd; USHORT SubCmd; PathParameter PathCmd[15];
                  JogParameter JogCmd; USHORT ClientHeartBeat; }          // 명령 입력
LogBuf          { bool Flag; MotorSt MotorStBuf[200][15]; MotorCmd MotorCmdBuf[200][15]; }
```

> ⚠️ `ClientHeartBeat` 필드는 TMC에 남아있지만 **현재 펌웨어에서 사용하지 않음**(heartbeat 워치독 제거됨). 잔재이며 무시해도 됨(정리하려면 TMC에서 제거 후 재생성).

## 5. 명령 프로토콜 (`ClientCommand`, USHORT)
- **MainCmd**: `ControlDrive=1`, `SetOperationMode=2`, `PathMotion=3`, `JogModeMove=4`
- **SubCmd** (MainCmd에 따라 의미 달라짐):
  - ControlDrive → `EnableSequence=1`, `QuitStep=2`, `FaultReset=3`
  - SetOperationMode → `Csp=1`, `Csv=2`, `Cst=3` (→ CiA402 8/9/10)
  - PathMotion → `TrapezoidalProfile=1`, `QuinticPolynomial=2`, `PlayPath=3`, `Reciprocate=4`, `SteppingOffsetMove=5`, `StopHoldAndResetPath=6`, `Pause=7`, `Resume=8`
  - JogModeMove → `Positive=1`, `Negative=2`, `Stop=3`

## 6. 사이클 동작 (`CycleUpdate`, 매 태스크 주기)
1. `CheckOrders()` (ADS indication/confirmation 처리)
2. `ServerToClient.DataMotorSt ← MotorStArea.Data` (memcpy)
3. `ApplyMainCmdMotorOutputs()` — MainCmd/SubCmd 디스패치
4. `ApplySafetyOverrides()` — **안전 오버라이드(최종 결정권)**
5. `ServerToClient.DataMotorCmd ← MotorCmdArea.Data` (memcpy)
6. `UpdateLoggingBuffer()` — 링버퍼 기록
7. 100사이클마다 `SubmitAdsReadReq()` (자동생성 샘플 유지)

## 7. 명령별 동작 상세
- **ControlDrive** (모터별 `nWcState`면 skip):
  - `EnableSequence`: StatusWord 상태비트(0x27/0x23/0x21)에 따라 `0x0F/0x07/0x06` 단계 진행. Fault 비트 있으면 skip. `TargetPosition=ActualPosition`으로 점프 방지.
  - `QuitStep`: Operation Enabled→하위로 한 스텝씩 복귀.
  - `FaultReset`: `nErrorCode != 0`일 때만 ControlWord에 `0x80` OR.
- **SetOperationMode**: 활성 전체 모터 `nModeOfOperation`에 CSP/CSV/CST 기록(ControlWord는 안 건드림).
- **JogModeMove (현재 = 속도/CSV 제어)**:
  - 방향=SubCmd, 대상축=`JogCmd.nJogID`, 속도크기=`JogCmd.nJogTick`.
  - 드라이브가 **CSV(9)** 모드일 때만 적용(`nModeOfOperationDisp != 9`이면 무시), `nWcState`면 무시.
  - `Positive`→`nTargetVelocity=+tick`, `Negative`→`-tick`, `Stop`→`0`.
  - **단일 축 정책**: jog 명령 시 선택 축 외 **모든 활성 축은 `nTargetVelocity=0`**. → Stop 없이 다른 축 jog하면 이전 축 자동 정지.
- **PathMotion**: `ApplyPathMotion()`이 해석적 궤적(경로버퍼 없음) 수행. Trapezoidal/Quintic/Reciprocate/SteppingOffset + PlayPath/Pause/Resume/StopHoldAndResetPath.

## 8. 안전 로직 (`ApplySafetyOverrides`, 명령 후 최종 적용)
- 모터별 **`nWcState == true`(전원 off / EtherCAT 통신 끊김)** → 해당 모터 `ControlWord=0x06(Shutdown)`, `ModeOfOperation=0` 강제.
- (이전에 있던 ADS heartbeat 워치독은 **제거됨**.)

## 9. 로깅 버퍼 (`UpdateLoggingBuffer`)
- 깊이 200 링버퍼. 매 사이클 상태/명령 1행을 `LogBuf.MotorStBuf[idx]`/`MotorCmdBuf[idx]`에 기록.
- `Flag`: `idx<99 → false`, `idx==99 → true`, `idx<199 → true`, `else false`. idx는 modulo 200.
- 클라이언트가 `Flag`로 안정적인 반쪽을 읽어가는 구조. (실제 CSV 저장은 `logging/` 폴더 — 클라이언트 측 산출물)

## 10. 빌드 / 배포 절차
1. TwinCAT XAE(Visual Studio)로 해당 `.sln`/`.slnx` 열기 (활성: `TcLowerBodyControl.slnx`).
2. 플랫폼 **TwinCAT OS (x64-E)**, 구성 **Release**로 C++ 모듈 빌드.
3. TMC 변경 시 반드시 **TMC Code Generation** 후 `ControllerServices.h` 갱신 확인.
4. TwinCAT으로 타깃에 **Activate Configuration** → 런타임 실행.

> ⚠️ `Module.h`/`Module.cpp`는 수기 코드. `ControllerServices.h`, `.tmc`, `.vcxproj`는 자동생성/구성 파일이라 임의 수정 금지(데이터영역 변경은 TMC 편집 후 재생성).

## 11. ADS 클라이언트 연동
- 클라이언트는 ADS 심볼로 `ClientToServer.*`에 명령 쓰기, `ServerToClient.*`/`LogBuf.*` 읽기.
- AMS NetId / ADS Port(관측값 350)로 접속. 심볼 경로 예: `TIXC^Controller^Controller_Obj^ClientToServer^MainCmd`.
- 표준 시퀀스 예: ①`SetOperationMode(Csp/Csv)` → ②`ControlDrive(EnableSequence)` 반복으로 Operation Enabled 도달 → ③`JogModeMove`/`PathMotion` 명령.

## 12. 알려진 이슈 / 주의사항
1. **`nJogID` 해석이 0-based/1-based 혼재**: `0~14`는 0-based, `15`만 1-based로 처리되어 `1~14` 구간 해석이 애매함. 단일 규칙으로 정리 권장.
2. **`ClientHeartBeat` 필드 미사용**(heartbeat 제거). TMC 잔재.
3. **모드 전제**: Jog는 드라이브가 CSV(9)일 때만 동작. `SetOperationMode(Csv)`를 먼저 보내야 함. CSP에서 jog 보내면 무시됨.
4. **Enable 전제**: `JogModeMove`/`SetOperationMode`는 ControlWord를 만들지 않음. `ControlDrive(EnableSequence)`로 Operation Enabled를 먼저 만들지 않으면 모터 무동작.
5. **프로젝트 중복**: lower가 `TcLowerBodyControl`(활성)과 `TcLowerBodyController`(구) 두 개 존재 + `TwinCAT Project1` 잔재. 실제 배포 대상 명확화 필요.
6. **ControlWord/ModeOfOperation=0 상황**: 초기/미설정 상태이거나(명령 안 보냄), `nWcState` 시 안전 오버라이드로 강제 0(mode)·6(CW).

## 13. 인수인계 체크리스트
- [ ] 실제 런타임에 Activate되는 프로젝트/타깃(AMS NetId) 확인
- [ ] ADS Port·심볼 경로(클라이언트 접속 파라미터) 공유
- [ ] EtherCAT 토폴로지 / 드라이브 ↔ `MotorStArea`/`MotorCmdArea[i]` 매핑표(축 i ↔ 물리 모터) 공유
- [ ] CiA402 enable 시퀀스 / 모드 전환 운영 절차 공유
- [ ] 미사용 자산 정리 여부 결정(`ClientHeartBeat`, `TwinCAT Project1`, 구 lower 프로젝트)
- [ ] `nJogID` 인덱싱 규칙 확정
