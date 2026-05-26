# STM32H533RETX BLDC FOC MicroMouse

이 프로젝트는 고성능 ARM Cortex-M33 코어를 탑재한 STM32H533RET6 보드를 기반으로 제작된 전문가용 마이크로마우스(MicroMouse)입니다. 기존의 스테핑 모터나 코어리스 DC 모터를 넘어서, **3상 BLDC 모터와 고해상도 자기식 엔코더(Magnetic Encoder)를 결합한 FOC(Field-Oriented Control)** 를 구현하여 극한의 주행 응답성과 부드러운 가감속 성능을 목표로 합니다. 여러 개의 다채널 ToF 센서와 6축 IMU를 융합한 정밀한 미로 탐색 및 자세 보정 알고리즘이 탑재되어 있습니다.

## 주요 하드웨어 구성 (Hardware Components)
* **MCU**: STM32H533RET6 (ARM Cortex-M33, 250MHz, TrustZone 지원 고성능 마이크로컨트롤러)
* **모터 드라이버**: DRV8316CRQ1 (Texas Instruments, 고신뢰성 3-Phase BLDC 전용 드라이버, SPI 파라미터 설정 지원)
* **자기식 엔코더**: MT6701 (MagnTek, 14-bit Absolute Magnetic Encoder, 회전자 전기각 파악용)
* **IMU 센서**: LSM6DS3TR-C (6축 가속도 및 자이로 센서)
* **거리/벽 센서**: VL53L4CX (다중 타겟 인식이 가능한 STMicro 고정밀 ToF 센서)
* **디스플레이 및 UI**: ST7789 TFT LCD 및 사용자 조작 스위치 (`menu.c`, `switch.c`)

---

## 코어 제어 기술: Field-Oriented Control (FOC)

이 마이크로마우스의 핵심은 하드웨어 타이머와 고해상도 엔코더를 이용해 3상 모터를 직류 모터처럼 손쉽고 정밀하게 제어하는 **독자적인 FOC (Field-Oriented Control) 제어기**(`foc.c`)입니다.

* **초정밀 피드백 획득 (`mt6701.c`)**:
  * 각 바퀴 모터 축에 장착된 MT6701 엔코더 데이터를 실시간으로 읽어와 회전자(Rotor)의 물리적 위치와 전기각(Electrical Angle)을 정밀하게 계산합니다.
* **전류/토크 제어 루프 (`foc.c`)**:
  * Clarke 변환과 Park 변환을 거쳐 3상(U, V, W) 전류/전압 지령을 직류 성분인 d-q 좌표계로 변환합니다.
  * $I_q$(토크 전류)와 $I_d$(자속 전류)를 개별적으로 PID 제어하며, 산출된 전압 벡터를 공간 벡터 PWM (SVPWM)을 통해 DRV8316으로 인가하여 발열과 진동을 최소화하면서 토크를 극대화합니다.
* **고속 응답성 보장**:
  * 마이크로초 단위로 타이트하게 구성된 제어 루프를 통해 가감속 중 슬립 현상을 억제하고 목표 속도에 즉각적으로 도달합니다.

---

## 주행 및 미로 탐색 알고리즘 (Driving & Navigation)

미로 내의 벽을 스캔하여 차체의 자세를 유지하고 목표 지점까지 최적의 모션으로 주행하는 시스템이 `drive.c`와 `sensor.c`에 구현되어 있습니다.

### 1. 다중 센서 융합 기반 미로 인식 (`sensor.c`)
* **VL53L4CX ToF 다중 폴링 (`vl53l4cx.c`)**:
  * 전방 및 45도 좌우 방향을 바라보는 ToF 센서를 지속적으로 트리거하여, 밀리미터(mm) 단위로 벽과의 거리를 측정합니다.
  * 벽의 유무를 판단할 뿐만 아니라 벽과의 평행도를 계산하여 차체가 미로의 정중앙을 유지하며 달리게 하는 **자세 보정(Wall Following)** 의 핵심 지표로 사용됩니다.
* **LSM6DS3TR-C 자이로 오도메트리 (`lsm6ds3tr-c.c`)**:
  * 고속 주행 시 휠 슬립으로 인해 발생하는 엔코더 오도메트리 오차를 자이로 센서의 Yaw 적분 값을 통해 상호 보완(Sensor Fusion)하여 직선 및 회전 궤적의 신뢰도를 높입니다.

### 2. 가속 프로파일 및 조향 제어 (`drive.c`)
* **PD 조향 제어**:
  * 목표 기준 속도(Target Speed)를 바탕으로, 좌우 벽과의 거리 오차 및 자이로 각속도를 입력받아 좌우 모터의 속도 지령값을 미세하게 조정합니다.
* **턴/코너링 모션 프로파일**:
  * 미로의 90도/180도 코너를 고속으로 돌파하기 위해 가속, 등속, 감속 구간을 수학적으로 계획한 모션 프로파일(Trapezoidal / S-Curve) 기반 턴 동작을 구동합니다.

---

## 하드웨어 인터페이스 및 시스템 최적화

### 1. DRV8316 스마트 모터 드라이버 제어 (`drv8316crq1.c`)
* SPI 통신을 통해 드라이버 내부에 접근하여, 보호 회로(OVP, OCP, OTP) 설정 및 PWM 데드타임(Dead-time), 슬루율(Slew rate) 등을 모터 하드웨어 특성에 맞게 동적으로 구성합니다.
* 에러 발생 시(`error.c`) 드라이버의 Fault 레지스터를 파악하여 LCD에 상태를 표시하고 전력을 차단하는 안전 정지를 수행합니다.

### 2. 메모리 및 데이터 관리 (`flash.c`, `icache.c`)
* 성능 최적화를 위한 **ICACHE (Instruction Cache)** 활성화(`icache.c`)를 통해 코어의 연산 지연을 줄였습니다.
* PID 튜닝 파라미터나 미로 탐색 맵(Map) 데이터를 비휘발성으로 보존하기 위해, MCU의 플래시 메모리 특정 구역을 EEPROM처럼 활용하는 Flash Read/Write 로직(`flash.c`)이 포함되어 있습니다.

## 핵심 코드 구조 (Directory Structure)
* `Core/Src/`: 시스템 클럭 설정, GPDMA, 하드웨어 타이머(LPTIM, TIM) 및 주요 주변장치(SPI, I2C, ADC) 초기화.
* `Motion/Src/foc.c`: 3상 BLDC 모터의 핵심인 공간 벡터 제어(SVPWM) 및 전류/속도/위치 PID 연산.
* `Motion/Src/drive.c`: 턴 궤적 생성, 벽 추종(Wall Following) 및 주행 상태 머신(가속/감속/정지).
* `Motion/Src/sensor.c`: 센서 융합 처리(ToF, IMU) 메인 로직.
* `Motion/Src/motor.c`: 물리적 바퀴(좌/우측 모터)에 대한 상위 제어 추상화 레이어.
* `Devices/Src/`: DRV8316 모터 드라이버, MT6701 엔코더, VL53L4CX 센서, LSM6DS3TR-C IMU 등 주요 하드웨어 IC별 통신 및 제어 드라이버.
