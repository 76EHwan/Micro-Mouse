# Micro Mouse (v1.10.0)

## HW Environment
* **MCU:** STM32H533RET6
* **BOARD:** Custom PCB (Self-made)
* **MOTOR:** 1515RB 7.4v or 1215RB 7.4v (Coreless Brushless Motor)
	* 1515RB: https://www.alibaba.com/product-detail/Hot-Sale-High-Speed-Coreless-Brushless_62443555830.html?spm=a27aq.27095423.1978240560.7.78376b643QDjiv
	* 1215RB: https://www.alibaba.com/product-detail/12mm-Brushless-Motor-7-4V-High_1601379472009.html?spm=a27aq.27095423.1978240560.1.78376b643QDjiv 
* **DRIVER:** DRV8316CRQRGFRQ1 (TI)
* **SENSOR:** 
	* **IMU:** LSM6DS3TR-C (6-axis)
	* **Encoder:** MT6701QT-STD (Magnetic)
	* **ToF:** VL53L4CX
* **DISPLAY:** ST7789 1.14 LCD

## Dev Environment
* **IDE:** STM32CubeIDE 1.18.0
* **F/W Library:** STM32Cube FW_H5 V1.5.1
* **Debugger:** ST-Link V2

## Comment
대학교 1학년 때 처음 라인트레이서 로봇을 만들기 시작해 올해로 약 5년차가 되었다.

그동안 로봇을 만들면서 느낀점은 다른 교육용으로 널리 쓰이는 로봇들은 부품이 싼 반면,
동아리에서 만드는 로봇들은 부품이 비싸, 초보자들이 로봇을 만들기에 진입 장벽이 높다고 생각한다.

기존 Classic-size Micro mouse에 널리 들어가는 FAULHABER 1717 DC 모터만 해도
세트당 50만원은 가볍게 넘어간다. 그래서 기존에 있던 모터를 떼오던가, 몇 년 동안 썩힌 모터를 꺼내어 사용해왔다.
나는 기존에 있는 모터를 떼오던 굴레를 없애기 위해, 기존 모터와 성능은 비슷하면서 싼 모터들을 찾았다.

그러다가 처음 눈에 들어온 것이 ORBRAY사의 CRS17-1801이다. 1717보다 사이즈가 조금 크지만, 토크 부분에서 1717을 능가한다.
하지만 견적을 내었을 때 개당 가격이 10만원대라 기각되었다.

그 뒤에 찾은 것이 1718RB Coreless bldc 모터이다. Alibaba에서 20달러 내외로 판매하는 것을 찾았고, 좀더 검색해보니
좀더 크기가 작으면서 성능이 비슷한 1515RB 모터를 찾았다. 개당 가격은 약 USD 18.0로 CRS17-1801보다 1/4 가격이다.













