# Dispositivo medidores de  CO2  usando software y hardware libre para monitorear la calidad del aire

___
## Contenido

1. [**Objetivo**](#Objetivo)
2. [**Intro**](#Introducción)
3. [**Metodologia**](#Metodología)
4. [**Evaluación**](#Evaluación)
5. [**Conclusiones**](#Conclusiones)
6. [**Constrúyelo**](#Constrúyelo)
7. **Trabajo futuro**
8. **Referencias**

___

## Objetivo

Caracterizar sensores de CO2 disponibles en el mercado mexicano y proporcionar
toda la información necesaria para construir tu propio dispositivo medidor de CO2 usando
software y hardware libre.


## Introducción

Cada día hay más evidencia de que el SARS-CoV-2 se transmite por medio de aerosoles.
En el proceso de respiración humana se expira CO2,  por lo que medir
el nivel de CO2 en un espacio nos da una idea de la ventilación de un espacio
y nos ayuda a tomar acciones para
reducir el riesgo de transmisión del SARS-CoV-2.

Una de las
[recomendaciones principales](https://unamglobal.unam.mx/pese-a-lo-que-nos-habian-dicho-la-covid-19-si-se-transmite-por-aire-y-la-ventilacion-es-crucial-para-protegernos/?fbclid=IwAR0sW6vuGHWTH13rlhkBLLkxjZxu5v7fGgK68bcxknCwfhoOIFyJ7NPaWW8)
es ventilar los espacios
cuando la concentración de CO2 sea mayor a 700 ppm.


## Metodología
Se diseñó un experimento para medir el tiempo de respuesta (TR)
y el error respecto a un dispositivo de referencia. El experimento
consiste en colocar el sensor a caracterizar y un sensor de referencia (Fluke 975)
en una caja (casi)hermética. El experimento inicia cuando se produce CO2 mezclando
vinagre y bicarbonato de sodio (Figura 1).


![800ppm](https://github.com/Dispositivos-Edificio-Bioclimatico/co2/blob/6793b8df06d98f5591a92e80fcb6b1445978810d/sensores_vco2/img/800ppm.png "800ppm")

El TR se define como el tiempo transcurrido  en que el cambio de la derivada de la concentración
respecto al tiempo es menor a 2 ppm, como se ve en la  Figura 2.

La prueba se deja por una hora para que alcance un estado permanente y se obtiene el promedio temporal
de los últimos 10 minutos tanto del sensor en caracterización como del dispositivo de referencia. El
error es el valor absoluto de la diferencia de ambas medidas.

La prueba se repite a diferentes concentraciones y se promedia el TR y el error de cada sensor.





# Evaluación


Los sensores que hemos probado son:

1. cjmcu811
2. cdm4160
3. t3022
4. scd30
5. sen0220
6. s8lp
7. sen0219

En una prueba de concentración se colocan varios sensores,
para cada sensor a una concentración dada, se calcula el error, el
tiempo de respuesta TR, y el valor de la medición en estado permanente
del sensor M y también del dispositivo de referencia D_r.

|         Sensor         |   Error |   TR  |    M |   D_r |
|:-----------------------|--------:|------:|-----:|------:|
|                        |   [ppm] | [min] |[ppm] | [ppm] |
| scd30                  |      37 |     3 |  762 |   800 |
| sen0220                |       3 |     1 |  803 |   800 |
| s8lp                   |      27 |     1 |  827 |   800 |
| cjmcu811               |    3105 |    13 | 3906 |   800 |
| t3022                  |      55 |     8 |  855 |   800 |



Se realizaron pruebas a diferentes concentraciones, alrededor de 500, 600, 700, 800 y 900 ppm
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
En esta sección se enlistan los sensores que consideramos adecuados indicando el fabricante, el precio en usd
 el tipo de comunicación implementado en el programa (Comm),
el voltaje de operación (Vin), el voltaje de comunicación (Vcom) la tecnología de medición (Tec), el número de canales (Canales),
las variables que mide (Variables)


| sensor    |  Fabricante | Precio |   Comm |   Vin     |    Vcom |   Tec  |   Canales  | Variables  |
| :-----    |  :--        | :--:   |  :--:  | :---:     | :---:   |  :---: | :-------:  | :-------:  |
|           |             | $usd   |        |  [V]      |  [V]    |        |            |            |
| s8lp      |  Senseair   |  44    |   UART |   Vin     |  5.0    |   NDIR |      1     | ppm        |
| scd30     |  Sensirion  |  61    |   I2C  | 4.5-5.25  |  3.3    |  NDIR  |      2     | ppm, oC, HR|



## Constrúyelo
