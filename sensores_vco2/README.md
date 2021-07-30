# Sensores para medir el CO2 en edificaciones

## Objetivo
Caracterizar sensores de CO2 disponibles en el mercado mexicano y proporcionar
toda la información necesaria para construir tu propio medidor de CO2 usando
software y hardware libre.


## Introducción

Cada día hay más evidencia de que el SARS-CoV-2 se transmite por medio de aerosoles.
En el proceso de respiración humana se expira CO2,  por lo que medir
el nivel de CO2 en un espacio nos da una idea de la ventilación de un espacio
y nos ayuda a tomar acciones para
reducir el riesgo de transmisión del SARS-CoV-2.


## Metodología
Se diseñó un experimento para medir el tiempo de respuesta (TR)
y el error respecto a un dispositivo de referencia. El experimento
consiste en colocar el sensor a caracterizar y un sensor de referencia (Fluke 975)
en una caja (casi)hermética. El experimento inicia cuando se produce CO2 mezclando
vinagre y bicarbonato de sodio.

El TR se define como el tiempo transcurrido  en que el cambio de la derivada de la concentración
respecto al tiempo es menor a 2 ppm, como se ve en la figura siguiente.

![800ppm](../img/800ppm.png?raw=true "800ppm")


en una caja hermética

Los sensores se prueban en una caja hermética a diferentes concentraciones cd CO2 y
se analiza el transitorio de la respuesta y el error una vez alcanzado el estado  
permanente. Se utiliza el Fluke 975 como dispositivo de referencia.

## Sensores

Los sensores que hemos probado son:

1. cjmcu811
2. cdm4160
3. t3022
4. scd30
5. sen0220
6. s8lp
7. sen0219

## M'etricas

Se mide el tiempo de respuesta respecto al dispositivo de referencia
y el error en estado permanente del experimento.

### Tiempo de respuesta

El tiempo de respuesta (TR) respecto al dispositivo de referencia (D_r) est'a
definido como el tiempo que tarda el sensor en alcanzar el 90% del valor
del dispositivo de referencia en estado permanente:

TR = t(D_r ) - t( S)

donde:

TR es el tiempo de respuesta del sensor S,

t(D_r) es el tiempo en el que se alcanz'o el 90% del valor
del dispositivo de referencia,

t(S) es el tiempo en el que el sensor en prueba alcanz'o
el 90% del valor del dispositivo de referencia.

### Estado permanente

Se define como valor en estado permanente cuando el promedio de
la pendiente es XXX

### Error en estado permanente

Una vez que el experimento ha alcanzado el estado permanente, se calcula el error

E =



# Resultados



| Sensor |   < M > | TR     | < Fluke > |  Error  |
|:-------| :-----  |:---    | :--:      | :--:    |
|        |  [ppm]  | MM:SS  |   [ppm]   |  [ppm]  |
| scd30  |  858    | 07:22  |  893.9    |   35.9  |
| t3022  |         |        |           | |  
|sen0220 |         |        |           | |
| s8lp   |         |        |           | |
