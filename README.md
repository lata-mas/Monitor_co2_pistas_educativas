# Dispositivo medidor de CO2 para monitorear la calidad del aire construido con hardware y software libre

(Click here for english version)[]
___
## Contenido

1. [**Objetivo**](#Objetivo)
2. [**Introducción**](#Introducción)
3. [**Metodologia**](#Metodología)
4. [**Sensores**](#Sensores)
5. [**Conclusiones**](#Conclusiones)
6. [**Constrúyelo**](#Constrúyelo)
7. [**Encárgalo**](#Encárgalo)
8. [**Pendientes**](#Pendientes)
9. [**Autores**](#Autores)
1. [**Referencias**](#Referencias)

___

## Objetivo

El objetivo general es diseñar un dispositivo medidor de CO2 para medir
la calidad del aire que sea confiable (con un error menor a 50 ppm) y lo más
barato, usando software y hardware libre.



## Introducción




Se diseñó y construyó un dispositivo  medidor de CO2 buscando minimizar el precio total
utilizando software y hardware libre. El dispositivo está basado en el
esp8266 lolin y cuenta con una   una batería recargable usb
que alimenta al esp8266 y al sensor.

Se diseñó un PCB y un programa en Arduino
que funciona con cualquiera de los sensores presentados en [**Conclusiones**](#Conclusiones)
siguiendo el esquemático de conexión correspondiente de cada sensor.
El dispositivo tiene la opción de publicar en la plataforma ThingsBoard y
tener un tablero en línea y  descargar los datos para su posterior análisis o visualización.

En la sección [**Constrúyelo**](#Constrúyelo) se presenta el esquemático de conexión y el programa, para
este caso se usa el sensor s8lp.

## Metodología
Se diseñó un experimento para medir el tiempo de respuesta (TR)
y el error respecto a un dispositivo de referencia (Fluke 975). El experimento
consiste en colocar el sensor a caracterizar y el dispositivo de referencia    
en una caja (casi) hermética. El experimento inicia cuando se produce CO2 mezclando
vinagre y bicarbonato de sodio (Figura 1) y se mide la concentración de CO2    
a lo largo del tiempo.


![800ppm](https://github.com/Dispositivos-Edificio-Bioclimatico/co2/blob/6793b8df06d98f5591a92e80fcb6b1445978810d/sensores_vco2/img/800ppm.png "800ppm")

El TR se define como el tiempo transcurrido  en que el cambio
de  la concentración de CO2
respecto al tiempo es menor a 2 ppm. En  la  Figura 2 se muestra
la tasa de cambio para el sensor s8lp y se puede apreciar que el TR es alrededor de 5 minutos.

La prueba dura una hora, y se asegura que se alcance un estado permanente. Una vez
que se ha alcanzado el estado permanente  se obtiene el promedio temporal
de los últimos 10 minutos tanto del sensor en caracterización como del dispositivo de referencia. El
error es el valor absoluto de la diferencia de ambas medidas.

La prueba se repite a diferentes concentraciones y se promedia el TR y el error de cada sensor.





# Sensores


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


El dispositivo está basado en el ESP8266 Lolin, y se seleccionó el sensor s8lp.




## Encárgalo  

Si no quieres construirlo, mándanos un correo, nosotros podemos hacerlo y te lo enviamos
con su hoja de caracterización, te lo enviamos armado. También podemos agregarle un plan de da

Si quieres tener varios dispositivos y monitorearlos via web y ser capaz de descargar
los datos, te podemos ofrecer un plan de monitoreo con un tablero y una libreta en Python
y Jupyter para descargar los datos.

[correo:co2@ier.unam.mx](mailto:co2@ier.unam.mx)

Costos del dispositivo incluyendo IVA.
| Cantidad    |  Precio      |
| :-----      |  :--         |
|             |  MXN         |    
| 1 - 4       |  XXX         |         
| 5-10        |  XXX         |
| + 10        |  XXX         |  


## Pendientes    

* [ ] 1 led RGB en lugar de tres
* [ ] Precios y materiales por sensor
* [ ] Vin a 5V para el esp32
* [ ] PCB universal para sensores seleccionados
* [ ] No usar jumper
* [ ] Carcasa con protección al sensor, pantalla y menor tamaño
* [ ] Integrar alarma sonora
* [ ] Visibilidad del nivel de bateria
* [ ] Configurable en 2 o 3 redes
* [ ] Incluir referencias bibliogr'aficas
* [ ] Que el scd30 despligue valores de oC, HR y ppm
* [ ] TR al 90% comenzando en 420 ppm


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
