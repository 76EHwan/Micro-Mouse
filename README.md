# STM32H533RETX BLDC FOC MicroMouse

이 프로젝트는 고성능 ARM Cortex-M33 코어를 탑재한 STM32H533RET6 보드를 기반으로 제작된 전문가용 마이크로마우스(MicroMouse)입니다. 기존의 스테핑 모터나 코어리스 DC 모터를 넘어서, **3상 BLDC 모터와 고해상도 자기식 엔코더(Magnetic Encoder)를 결합한 FOC(Field-Oriented Control)** 를 구현하여 극한의 주행 응답성과 부드러운 가감속 성능을 목표로 합니다. 4채널 ToF 센서로 벽까지의 거리를 측정하고, 6축 IMU로 Yaw 각도를 추정합니다.

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

---

## 센서 및 주행 (Sensing & Driving)

센서 처리는 `sensor.c`, 주행 속도 제어는 `drive.c`에 구현되어 있습니다.

### 1. 센서 처리 (`sensor.c`)
* **VL53L4CX ToF 다중 폴링 (`vl53l4cx.c`)**:
  * 4개의 ToF 센서(L, CL, CR, R)를 타이머 인터럽트로 순차 폴링하여, 밀리미터(mm) 단위로 벽과의 거리를 측정합니다.
* **LSM6DS3TR-C 자이로 (`lsm6ds3tr-c.c`)**:
  * 자이로 Z축 각속도에서 오프셋을 보정한 뒤 적분하여 차체의 Yaw 각도(0~360°)를 계산합니다.

### 2. 가감속 프로파일 (`drive.c`)
* 목표 속도(Target Speed)까지 설정된 가속도/감속도로 현재 속도를 선형적으로 증감시켜 좌우 모터의 속도 지령값을 생성합니다.

---

## 하드웨어 인터페이스 및 시스템 최적화

### 1. DRV8316 스마트 모터 드라이버 제어 (`drv8316crq1.c`)
* SPI 통신을 통해 드라이버 내부에 접근하여, 보호 회로(OVP, OCP) 설정 및 슬루율(Slew rate), PWM 모드 등을 구성하고, 설정된 레지스터 값을 다시 읽어 검증합니다.

### 2. 성능 최적화 (`icache.c`)
* **ICACHE (Instruction Cache)** 활성화(`icache.c`)를 통해 코어의 연산 지연을 줄였습니다.

## 핵심 코드 구조 (Directory Structure)
* `Core/Src/`: 시스템 클럭 설정, GPDMA, 하드웨어 타이머(LPTIM, TIM) 및 주요 주변장치(SPI, I2C, ADC) 초기화.
* `Motion/Src/foc.c`: 3상 BLDC 모터의 핵심인 공간 벡터 제어(SVPWM) 및 d-q 전류 PI 연산.
* `Motion/Src/drive.c`: 목표 속도 기반 가감속 프로파일.
* `Motion/Src/sensor.c`: ToF 거리 측정 및 IMU Yaw 각도 계산.
* `Motion/Src/motor.c`: 좌/우측 모터 구동 시작 및 오픈루프 테스트 구동(6-step, SVPWM).
* `Devices/Src/`: DRV8316 모터 드라이버, MT6701 엔코더, VL53L4CX 센서, LSM6DS3TR-C IMU 등 주요 하드웨어 IC별 통신 및 제어 드라이버.
