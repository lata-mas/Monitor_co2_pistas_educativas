EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Dispositivo_CO2"
Date "2021-10-08"
Rev "1.1"
Comp "IER-UNAM"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L ESP8266:NodeMCU_1.0_(ESP-12E) U2
U 1 1 615F62EE
P 5000 3500
F 0 "U2" H 5000 4693 60  0000 C CNN
F 1 "NodeMCU_1.0_(ESP-12E)" H 5000 4587 60  0000 C CNN
F 2 "ESP8266:NodeMCU-LoLinV3" H 5000 4481 60  0000 C CNN
F 3 "" H 4400 2650 60  0000 C CNN
	1    5000 3500
	0    1    1    0   
$EndComp
$Comp
L Device:LED_RAGB D1
U 1 1 615FF3B4
P 2850 4800
F 0 "D1" H 2850 5200 50  0000 C CNN
F 1 "LED_RGB" H 2850 5300 50  0000 C CNN
F 2 "LED_THT:LED_D5.0mm-4_RGB_Wide_Pins" H 2850 4750 50  0001 C CNN
F 3 "~" H 2850 4750 50  0001 C CNN
	1    2850 4800
	-1   0    0    1   
$EndComp
$Comp
L Device:R R1
U 1 1 616048E6
P 3250 4600
F 0 "R1" V 3150 4650 50  0000 C CNN
F 1 "220" V 3150 4500 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 3180 4600 50  0001 C CNN
F 3 "~" H 3250 4600 50  0001 C CNN
	1    3250 4600
	0    1    1    0   
$EndComp
$Comp
L Device:R R1
U 1 1 61605002
P 3250 4800
F 0 "R1" V 3150 4850 50  0000 C CNN
F 1 "220" V 3150 4700 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 3180 4800 50  0001 C CNN
F 3 "~" H 3250 4800 50  0001 C CNN
	1    3250 4800
	0    1    1    0   
$EndComp
$Comp
L Device:R R1
U 1 1 616052D2
P 3250 5000
F 0 "R1" V 3150 5050 50  0000 C CNN
F 1 "220" V 3150 4900 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 3180 5000 50  0001 C CNN
F 3 "~" H 3250 5000 50  0001 C CNN
	1    3250 5000
	0    1    1    0   
$EndComp
$Comp
L Device:R R2
U 1 1 616055A5
P 5500 5550
F 0 "R2" V 5600 5550 50  0000 C CNN
F 1 "10k" V 5384 5550 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 5430 5550 50  0001 C CNN
F 3 "~" H 5500 5550 50  0001 C CNN
	1    5500 5550
	0    1    1    0   
$EndComp
Wire Wire Line
	4700 4300 4700 4600
Wire Wire Line
	4700 4600 3400 4600
Wire Wire Line
	3100 4600 3050 4600
Wire Wire Line
	3100 4800 3050 4800
Wire Wire Line
	3100 5000 3050 5000
Wire Wire Line
	4800 4300 4800 4800
Wire Wire Line
	4800 4800 3400 4800
Wire Wire Line
	5700 4300 5700 5000
Wire Wire Line
	5700 5000 3400 5000
Wire Wire Line
	5000 4700 5000 4300
$Comp
L Switch:SW_Push SW1
U 1 1 6160F77D
P 5000 5550
F 0 "SW1" H 5000 5835 50  0000 C CNN
F 1 "Button" H 5000 5744 50  0000 C CNN
F 2 "Button_Switch_THT:SW_DIP_SPSTx01_Slide_9.78x4.72mm_W7.62mm_P2.54mm" H 5000 5750 50  0001 C CNN
F 3 "~" H 5000 5750 50  0001 C CNN
	1    5000 5550
	1    0    0    -1  
$EndComp
$Comp
L buzzer3:buzzer3 U1
U 1 1 6161B2AF
P 3900 5550
F 0 "U1" V 3846 5422 50  0000 R CNN
F 1 "Buzzer_activo" V 3937 5422 50  0000 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 3900 5550 50  0001 C CNN
F 3 "" H 3900 5550 50  0001 C CNN
	1    3900 5550
	0    -1   1    0   
$EndComp
Wire Wire Line
	5300 5150 5300 4300
Wire Wire Line
	3950 5150 3950 5300
Wire Wire Line
	3950 5150 5300 5150
Wire Wire Line
	4800 5550 4800 4900
Wire Wire Line
	4800 4900 5100 4900
Wire Wire Line
	5650 5550 5650 4600
Wire Wire Line
	5200 4600 5200 4400
Wire Wire Line
	5200 5550 5250 5550
Wire Wire Line
	5400 4300 5400 5250
Wire Wire Line
	5400 5250 5250 5250
Wire Wire Line
	5250 5250 5250 5550
Connection ~ 5250 5550
Wire Wire Line
	5250 5550 5350 5550
Wire Wire Line
	4900 4800 4900 4300
Wire Wire Line
	3650 2500 3900 2500
Wire Wire Line
	4300 2700 4300 2500
Connection ~ 3900 2500
Wire Wire Line
	3900 2500 4300 2500
Wire Wire Line
	6950 4700 6950 4950
Wire Wire Line
	5000 4700 6950 4700
Wire Wire Line
	6850 4950 6850 4800
Wire Wire Line
	3900 5300 3900 2500
Wire Wire Line
	2650 4800 2650 4400
Wire Wire Line
	2650 4400 5200 4400
Connection ~ 5200 4400
Wire Wire Line
	5200 4400 5200 4300
Wire Wire Line
	4900 4800 6850 4800
$Comp
L co2_project-rescue:TM1637Display-rur U3
U 1 1 61609CBB
P 6800 5500
F 0 "U3" V 6700 5900 50  0000 R CNN
F 1 "Display" V 6900 6000 50  0000 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical" H 6800 5500 50  0001 C CNN
F 3 "" H 6800 5500 50  0001 C CNN
	1    6800 5500
	0    1    1    0   
$EndComp
Wire Wire Line
	5100 4900 5100 4850
Wire Wire Line
	5650 4600 5200 4600
Wire Wire Line
	6650 4950 6650 4850
Wire Wire Line
	6650 4850 5100 4850
Connection ~ 5100 4850
Wire Wire Line
	5100 4850 5100 4300
$Comp
L Connector:Conn_01x05_Male J1
U 1 1 6166421A
P 3300 2550
F 0 "J1" H 3550 2950 50  0000 C CNN
F 1 "5v" H 3550 2850 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x05_P2.54mm_Vertical" H 3300 2550 50  0001 C CNN
F 3 "~" H 3300 2550 50  0001 C CNN
	1    3300 2550
	1    0    0    -1  
$EndComp
Wire Wire Line
	3500 2350 3650 2350
Wire Wire Line
	3650 2350 3650 2500
Wire Wire Line
	3500 2750 3650 2750
Wire Wire Line
	3650 2750 3650 2600
$Comp
L s8part2:lp U5
U 1 1 61653887
P 7000 3650
F 0 "U5" H 7200 3650 118 0000 L CNN
F 1 "lp" H 6800 4200 118 0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x06_P2.54mm_Vertical" H 7000 3650 118 0001 C CNN
F 3 "" H 7000 3650 118 0001 C CNN
	1    7000 3650
	1    0    0    -1  
$EndComp
Wire Wire Line
	6500 3450 6350 3450
Wire Wire Line
	6350 3450 6350 4350
Wire Wire Line
	6350 4350 5600 4350
Wire Wire Line
	6500 3800 6450 3800
Wire Wire Line
	6450 3800 6450 4400
Wire Wire Line
	6450 4400 5500 4400
Wire Wire Line
	7000 2500 4300 2500
Connection ~ 4300 2500
Wire Wire Line
	7150 4000 7150 2600
Wire Wire Line
	5500 4400 5500 4300
Wire Wire Line
	6900 4450 5600 4450
Wire Wire Line
	5600 4300 5600 4350
Connection ~ 5600 4350
Wire Wire Line
	5600 4350 5600 4450
Connection ~ 5500 4400
Wire Wire Line
	7000 4000 7150 4000
Wire Wire Line
	6900 4650 5500 4650
Wire Wire Line
	5500 4400 5500 4650
Wire Wire Line
	7000 2500 7000 3150
Wire Wire Line
	7350 4300 7350 4150
Wire Wire Line
	7350 3150 7000 3150
Connection ~ 7000 3150
Wire Wire Line
	7000 3150 7000 3300
Wire Wire Line
	6750 4950 6750 4150
Wire Wire Line
	6750 4150 7350 4150
Connection ~ 7350 4150
Wire Wire Line
	7350 4150 7350 3150
Wire Wire Line
	3650 2600 3850 2600
Wire Wire Line
	4400 2600 4400 2700
Wire Wire Line
	4400 2600 7150 2600
Connection ~ 4400 2600
Wire Wire Line
	3850 5300 3850 2600
Connection ~ 3850 2600
Wire Wire Line
	3850 2600 4400 2600
$Comp
L sen0220:sen0220_scd30 U6
U 1 1 61668B07
P 7150 4500
F 0 "U6" H 7528 4554 50  0000 L CNN
F 1 "sen0220_scd30" H 7528 4463 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x07_P2.54mm_Vertical" H 7150 4500 50  0001 C CNN
F 3 "" H 7150 4500 50  0001 C CNN
	1    7150 4500
	1    0    0    -1  
$EndComp
Wire Wire Line
	7450 2600 7150 2600
Wire Wire Line
	7450 2600 7450 4300
Connection ~ 7150 2600
$Comp
L s8part1:s8 U4
U 1 1 6167FC23
P 6750 3650
F 0 "U4" H 6650 3700 118 0000 L CNN
F 1 "s8" H 6600 4200 118 0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x06_P2.54mm_Vertical" H 6750 3650 118 0001 C CNN
F 3 "" H 6750 3650 118 0001 C CNN
	1    6750 3650
	1    0    0    -1  
$EndComp
$EndSCHEMATC
