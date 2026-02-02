> [!NOTE]
> 이 프로젝트는 라즈베리 파이 5 (Linux Kernel 6.12+) 환경에서의 오디오 드라이버 실습용입니다.<br> 
> 소스 파일명은 `my_amp_max98357a_driver.c`이며 빌드 시 `my_amp_max98357a_driver.ko`가 생성됩니다.<br>
> 하드웨어 제어(GPIO) 권한이 필요하므로 `sudo` 권한으로 실행하거나 유저 권한 설정이 필요할 수 있습니다.<br>


# MAX98357A Linux ASoC Audio Driver for Raspberry Pi 5

라즈베리파이 5에서 **MAX98357A I2S Mono Amplifier**를 구동하기 위한 **리눅스 드라이버 모듈**입니다.

단순한 오디오 출력을 넘어, **Device Tree Overlay**를 직접 작성하여 라즈베리 파이 5의 대응하였으며, 커널 드라이버를 통해 앰프의 **Enable/Shutdown (GPIO 23)** 상태를 제어하여 팝 노이즈(Pop-noise)를 방지하고 전력 효율을 관리합니다.

## 주요 기능 (Features)
* **ASoC 프레임워크 구현:** Machine-Platform-Codec 구조에 맞춘 ALSA 드라이버 등록
* **Device Tree Overlay 적용:** RPi 5의 I2S 인터페이스(`&i2s`)와 GPIO 핀맵핑 커스터마이징
* **표준 ALSA 인터페이스:** `aplay`, `speaker-test`, `mpg123` 등 표준 리눅스 오디오 유틸리티 호환

## 하드웨어 연결 (Wiring)

MAX98357A는 I2S 통신을 사용하며, 라즈베리 파이 5의 40핀 헤더에 다음과 같이 연결합니다.

| MAX98357A Pin | Raspberry Pi 5 Pin | BCM (GPIO) | Function |
| :--- | :--- | :--- | :--- |
| **VIN** | Pin 1 or 17 | 3.3V or 5V | Power |
| **GND** | Pin 6 or 9 | GND | Ground |
| **LRC** | Pin 35 | GPIO 19 | LR Clock (Frame Sync) |
| **BCLK** | Pin 12 | GPIO 18 | Bit Clock |
| **DIN** | Pin 40 | GPIO 21 | Data In (SDO) |
| **SD_MODE** | **Pin 16** | **GPIO 23** | **Shutdown / Channel Select** |
| **GAIN** | - | - | Floating (12dB Default) |

> **Note:** `SD_MODE` 핀은 드라이버(GPIO 23)를 통해 제어됩니다. 하드웨어 저항 없이 연결 시 **Left Channel** 모드로 동작합니다. 

## 🛠️ 빌드 및 설치 (Build & Install)

## Device Tree 설정 
이 드라이버가 작동하려면 **Device Tree Overlay**를 통해 커널에 오디오 카드와 코덱 정보를 등록해야 합니다.

### 1.컴파일 및 설치
Device Tree 컴파일러(`dtc`)를 사용하여 `.dtbo` 파일을 만들고 시스템 폴더로 복사합니다.

```bash
# 1. dts를 dtbo로 컴파일
dtc -@ -I dts -O dtb -o my-amp-max98357a.dtbo my-amp-max98357a.dts

# 2. 시스템 오버레이 폴더로 복사
sudo cp my-amp-max98357a.dtbo /boot/firmware/overlays/
```

### 2.부트로더 설정 (`config.txt`)
`/boot/firmware/config.txt` 파일을 열어 오버레이를 활성화합니다.

```bash
sudo vim /boot/firmware/config.txt
```

파일 맨 아래에 다음 내용을 추가하고 재부팅합니다.

```ini
# I2S 활성화 및 커스텀 오버레이 로드
dtparam=i2s=on
dtoverlay=my-amp-max98357a
```

### 3. 재부팅

```bash
sudo reboot 
```

## 드라이버 올리기 insmod

### 1. 필수 패키지 설치
커널 헤더 파일이 필요합니다.
```bash
sudo apt install raspberrypi-kernel-headers
```

### 2. 컴파일 (Make)
```bash
make
```

### 3. 드라이버 로드 (Load Module)
```bash
sudo insmod my_amp_max98357a_driver.ko
```

### 4. 설치 확인
사운드 카드가 정상적으로 등록되었는지 확인합니다.
```bash
# 커널 로그 확인 (드라이버 등록 메시지)
dmesg | tail

# 사운드 카드 목록 확인
aplay -l
# 결과에 [My-Max98357A] 카드가 보여야 함
```

## 사용 예제 (Usage)
### 1. 기본 테스트 (WAV 재생)
단일 채널(Left) 테스트를 위해 `speaker-test`를 사용합니다.
* `-D hw:2,0`: 2번 카드(My-Max98357A) 직접 지정
```bash
speaker-test -D hw:2,0 -c2 -t wav -f S16_LE
```

### 2. MP3 음악 재생 (소프트웨어 믹싱)
하드웨어 저항 없이 연결된 경우(Left Channel Only), 스테레오 MP3 파일을 들으려면 소프트웨어적으로 채널을 섞어줘야(Mix) 합니다.
* `mpg123` 설치 필요 (`sudo apt install mpg123`)
* `plughw`: 샘플링 레이트 자동 변환 지원
* `--mix`: 스테레오 음원을 모노로 다운믹스

```bash
mpg123 -a plughw:2,0 --mix test_song.mp3
```

## 개발 환경 (Environment)
Hardware: Raspberry Pi 5 

OS: Raspberry Pi OS (64-bit)

Kernel: Linux 6.12.x

Audio Chip: Maxim MAX98357A (I2S Class D Amp)

## 라이선스 (License)
GPL v2
