# Dispositivo medidor de  CO2 para monitorear la calidad del aire construido con hardware y software libre

___
## Contenido

1. [**Objetivo**](#Objetivo)
2. [**Introducción**](#Introducción)
3. [**Metodologia**](#Metodología)
4. [**Evaluación**](#Evaluación)
5. [**Conclusiones**](#Conclusiones)
6. [**Constrúyelo**](#Constrúyelo)
7. [**Pendientes**](#Pendientes)
8. [**Autores**](#Autores)
9. [**Referencias**](#Referencias)

___

## Objetivo

Caracterizar sensores de CO2 disponibles en el mercado mexicano y proporcionar
toda la información necesaria para construir un dispositivo medidor de CO2 usando
software y hardware libre.


## Introducción

Cada día hay más evidencia de que el SARS-CoV-2 se transmite por medio de aerosoles[1,2].
En el proceso de respiración humana se expira CO2,  por lo que medir
el nivel de CO2 en un espacio nos da una idea de la ventilación de un espacio
y nos ayuda a tomar acciones para
reducir el riesgo de transmisión del SARS-CoV-2.

Una de las recomendaciones principales
es ventilar los espacios
cuando la concentración de CO2 sea mayor a 700 ppm[2].

Se diseñó  un dispositivo  medidor de CO2 buscando minimizar el precio total
utilizando software y hardware libre. El dispositivo está basado en el
esp8266 lolin alimentado por el puerto micro-usb por una batería recargable usb
que alimenta al esp8266 y al sensor. Se diseñó un PCB y un programa en arduino
que funciona con cualquiera de los sensores presentados en [**Conclusiones**](#Conclusiones)
siguiendo el esquemático de conexión correspondiente. 

## Metodología
Se diseña un experimento para medir el tiempo de respuesta (TR)
y el error respecto a un dispositivo de referencia. El experimento
consiste en colocar el sensor a caracterizar y un sensor de referencia (Fluke 975)
en una caja (casi)hermética. El experimento inicia cuando se produce CO2 mezclando
vinagre y bicarbonato de sodio (Figura 1).


![800ppm](https://github.com/Dispositivos-Edificio-Bioclimatico/co2/blob/6793b8df06d98f5591a92e80fcb6b1445978810d/sensores_vco2/img/800ppm.png "800ppm")

El TR se define como el tiempo transcurrido  en que el cambio
de la derivada de la concentración de CO2
respecto al tiempo es menor a 2 ppm. En  la  Figura 2 se muestra
la tasa de cambio para el sensor s8lp y se puede apreciar que el TR es alrededor de 5 minutos.

La prueba dura una hora, y se asegura que se alcance un estado permanente. Una vez
que se ha alcanzado el estado permanente  se obtiene el promedio temporal
de los últimos 10 minutos tanto del sensor en caracterización como del dispositivo de referencia. El
error es el valor absoluto de la diferencia de ambas medidas.

La prueba se repite a diferentes concentraciones y se promedia el TR y el error de cada sensor.





# Evaluación


Los sensores que se han probado son:

1. cjmcu811
2. cdm4160
3. t3022
4. scd30
5. sen0220
6. s8lp
7. sen0219

En una prueba a una concentración dada se colocan los  sensores en evaluación y
se calcula el error, el
tiempo de respuesta TR, y el valor de la medición en estado permanente
del sensor M y también del dispositivo de referencia D_r a la concentración dada.

|         Sensor         |   Error |   TR  |    M |   D_r |
|:-----------------------|--------:|------:|-----:|------:|
|                        |   [ppm] | [min] |[ppm] | [ppm] |
| scd30                  |      37 |     3 |  762 |   800 |
| sen0220                |       3 |     1 |  803 |   800 |
| s8lp                   |      27 |     1 |  827 |   800 |
| cjmcu811               |    3105 |    13 | 3906 |   800 |
| t3022                  |      55 |     8 |  855 |   800 |



Se realizan pruebas a diferentes concentraciones, alrededor de 500, 600, 700, 800 y 900 ppm
de CO2. Del conjunto de pruebas, para cada sensor se calcula
el promedio y la desviación estándard
del error ('Error','mean'), y ('Error','std'), respectivamente.
También se calcula el promedio y la desviación estándard  ('TR', 'mean')  y
('TR', 'std'), respectivamente.


| sensor   |   ('Error', 'mean') |   ('Error', 'std') |   ('TR', 'mean')  |   ('TR', 'std')  |
|:---------|--------------------:|-------------------:|------------------:|-----------------:|
|          |       [ppm]         |        [ppm]       |      [min]        |        [min]     |
| sen0220  |                  18 |                 26 |               3.2 |                2 |
| s8lp     |                  23 |                  6 |               2.8 |                2 |
| scd30    |                  41 |                  5 |               3.8 |                3 |
| t3022    |                  58 |                  3 |              16   |               10 |
| cjmcu811 |                3619 |                508 |              10.6 |                2 |


## Conclusiones

En esta sección se enlistan los sensores que consideramos adecuados presentando el error,
el tiempo de respuesta (TR), ambos con su desviación estándard,  fabricante,  precio en usd
 tipo de comunicación implementado en el programa (Comm),
 voltaje de operación (Vin),  voltaje de comunicación (Vcom),  tecnología de medición de CO2 (Tec),
  número de canales (Canales), y
  variables que mide el sensor (Variables).


| sensor    |  Error      |  TR      | Fabricante  | Precio |   Comm |   Vin     |    Vcom |   Tec  |   Canales  | Variables  |
| :-----    |  :--        |  :--:    |  :--        | :--:   |  :--:  | :---:     | :---:   |  :---: | :-------:  | :-------:  |
|           |  [ppm]      |  [min]   |             | $usd   |        |  [V]      |  [V]    |        |            |            |         
| s8lp      |  23 +- 6    | 2.8 +- 2 |  Senseair   |  44    |   UART |   5.0     |  5.0    |   NDIR |      1     | ppm        |
| scd30     |  41 +- 5    | 3.8 +- 3 |  Sensirion  |  61    |   I2C  | 4.5-5.25  |  3.3    |  NDIR  |      2     | ppm, oC, HR|



## Constrúyelo


El dispositivo está basado en el ESP8266 Lolin, y se utilizará el sensor s8lp.





## Pendientes    

* [ ] 1 led RGB en lugar de tres
* [ ] Precios y materiales por sensor
* [ ] Vin a 5V para el esp32
* [ ] PCB universal para sensores seleccionados
* [ ] No usar jumper
* [ ] Carcasa con protección al sensor, pantalla y menor tamaño
* [ ] Integrar alarma sonora
* [ ] Visibilidad del nivel de bateria
* [ ] Configurable en red
* [ ] Incluir referencias bibliogr'aficas
* [ ] Que el scd30 despligue valores de oC, HR y ppm


## Autores

En este trabajo fue iniciado por (orden alfabético):

* Giovanni Velázquez
* Guillermo Barrios
* Guillermo Ramirez
* Héctor Daniel Cortés

Y a la fecha han contribuido:


## Referencias

1.[Singing unmasked, indoors spreads COVID-19 through aerosols, new study confirms](https://www.colorado.edu/today/2020/09/17/singing-unmasked-indoors-spreads-covid-19-through-aerosols-new-study-confirms)

2.[Pese a lo que nos habían dicho, la COVID-19 sí se transmite por aire y la ventilación es crucial para protegernos](https://unamglobal.unam.mx/pese-a-lo-que-nos-habian-dicho-la-covid-19-si-se-transmite-por-aire-y-la-ventilacion-es-crucial-para-protegernos/?fbclid=IwAR0sW6vuGHWTH13rlhkBLLkxjZxu5v7fGgK68bcxknCwfhoOIFyJ7NPaWW8)
