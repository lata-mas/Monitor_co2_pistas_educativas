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
vinagre y bicarbonato de sodio (Figura 1).


![800ppm](https://github.com/Dispositivos-Edificio-Bioclimatico/co2/blob/d2ca1b8e98e3b86a8cee72be6eeb1e02ed23bd05/sensores_vco2/img/800ppm.png "800ppm")

El TR se define como el tiempo transcurrido  en que el cambio de la derivada de la concentración
respecto al tiempo es menor a 2 ppm, como se ve en la  Figura 2.

La prueba se deja por una hora para que alcance un estado permanente y se obtiene el promedio temporal
de los últimos 10 minutos tanto del sensor en caracterización como del dispositivo de referencia. El
error es el valor absoluto de la diferencia de ambas medidas.

La prueba se repite a diferentes concentraciones y se promedia el TR y el error de cada sensor.

## Sensores

Los sensores que hemos probado son:

1. cjmcu811
2. cdm4160
3. t3022
4. scd30
5. sen0220
6. s8lp
7. sen0219



# Resultados
En la siguiente tabla se presenta el promedio temporal en estado permanente
del sensor en prueba < M >, el tiempo de respuesta TR, el promedio temporal
en estado permanente del dispositivo de referencia < Fluke >  y el error absoluto,
que es el valor absoluto entre < M > y < Fluke >.

| sensor   |   ('Error', 'mean') |   ('Error', 'std') |   ('TR+', 'mean') |   ('TR+', 'std') |
|:---------|--------------------:|-------------------:|------------------:|-----------------:|
| sen0220  |             18.2364 |           25.5517  |           3.22468 |          2.01775 |
| s8lp     |             23.2094 |            6.24531 |           2.83337 |          2.11715 |
| scd30    |             40.5712 |            4.96174 |           3.8037  |          2.5656  |
| t3022    |             57.8272 |            3.43919 |          15.9704  |          9.92708 |
| cjmcu811 |           3618.76   |          507.741   |          10.6078  |          2.43256 |
