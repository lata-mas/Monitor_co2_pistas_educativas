# Dispositivo medidor de CO2 con sensores NDIR intercambiables para evaluar la ventilación natural


En este repositorio se encuentran los archivos necesarios para replicar el monitor de CO2 desarrollado en el Instituto de Energías Renovables de la UNAM, ubicado en Temixco, Morelos, México.





Los archivos para el PCB se encuentran en el folder **electric_design**. Los archivos KiCAD del esquema del PCB y el diagrama eléctrico se encuentran en el folder **pcb_src**. Los archivos para maquinar o cortar por láser el PCB se encuentran en el folder **pcb_gerber**. El diagrama eléctrico en formato PDF se encuentra en el folder **pcb_pdf**. Las plantillas para producir el PCB por impresión de pantalla en formato JPG se encuentran en el folder **pcb_jpg**. Adicionalmente, renders tridimensionales del PCB con las partes del monitor se encuentran en el folder **pcb_view3D**. Los archivos para producir la carcasa del monitor de CO2 se encuentran en el folder **hardcase_pdf**. Los archivos FreeCAD se encuentran en el folder **cad**, junto con archivos en PDF para calcar en el folder **hardcase_pdf**, y también archivos en formato STL y DXF en el folder **hardcase_printable**.

El programa, código fuente y binario, se encuentran en el folder **software**. En el folder **notebook** se encuentran libretas de Jupyter con los datos de la calibración hecha y la campaña experimental, los datos se encuentran en el folder **dat**.


# Lista de componentes
| Designador  | Componente                      | Cantidad | Costo (MXN) | Fuente de materiales |
|-------------|---------------------------------|----------|-------------|----------------------|
| ESP8266     | NodeMCU V3.0 Lolin              | 1        | 159.0       | [Steren](https://www.steren.com.mx/placa-de-desarrollo-nodemcu-esp8266.html) |
| Sensor CO2  | S8LP                            | 1        | 800.0       | [Digikey](https://www.digikey.com.mx/es/products/detail/senseair/004-0-0053/10416532) |
| Sensor CO2  | SEN0220                         | 1        | 1600.0      | [DFROBOT](https://www.dfrobot.com/index.php?route=product/product&product_id=1565) |
| Sensor CO2  | SCD30                           | 1        | 1067.0      | [DigiKey](https://www.digikey.com.mx/es/products/detail/sensirion-ag/SCD30/8445334) |
| Pantalla    | Pantalla de 4 dígitos           | 1        | 112.0       | [Seeed Studio](https://www.seeedstudio.com/Grove-4-Digit-Display.html?queryID=a52cccea41a634e8bc10114075018055&objectID=1651&indexName=bazaar_retailer_products) |
| LED         | LED RGB                         | 1        | 4.3         | [Seeed Studio](https://www.seeedstudio.com/5mm-Triple-Output-LED-RGB-Common-cathode-20-PC-p-623.html?queryID=996fc5e5da5e1e2ef55538094fc749c5&objectID=1920&indexName=bazaar_retailer_products) |
| BUZZER      | Zumbador activo                 | 1        | 27.7        | [Seeed Studio](https://www.seeedstudio.com/Grove-Buzzer.html?queryID=762ee5a017832256c2a6a486656facf6&objectID=1805&indexName=bazaar_retailer_products) |
| microUSB    | Adaptador microUSB              | 1        | 7.0         | [UElectronics](https://uelectronics.com/producto/adaptador-micro-usb-tipo-b-hembra-a-dip-pcb-5-pines/) |
| Botón       | Botón pulsador                  | 1        | 27.7        | [Newark](https://www.newark.com/philmore/30-10062/product-range/dp/43W7727?st=button) |
| Batería     | Banco de energía 2500 mAh       | 1        | 200.0       | [Newark](https://www.newark.com/multicomp/mc011332/power-bank-5-2ah-5vdc/dp/94AC6834?st=power%20bank) |
| R1          | Resistencia de 10 kOhm          | 1        | 2.5         | [Newark](https://www.newark.com/arcol/mra0207-10k-b-15ppm-ta/res-10k-0-10-250mw-axial/dp/79Y4556?st=resistor%2010kohm) |
| R2          | Resistencia de 220 Ohm          | 3        | 2.5         | [Newark](https://www.newark.com/multicomp-pro/mccfr0w4j0221a50/carbon-film-resistor-220-ohm-250mw/dp/58K5029?st=resistor%20220%20ohm) |
| Grove       | Cable groove                    | 1        | 2.5         | [Seeed Studio](https://www.seeedstudio.com/Grove-Universal-4-Pin-Buckled-5cm-Cable-5-PCs-Pack.html?queryID=5db340ba008fd0a5fd1f440a6d09659c&objectID=1701&indexName=bazaar_retailer_products) |
| Terminal    | Conector vertical de 4 pines GROVE 2mm | 1  | 2.5  | [Digikey](https://www.digikey.com.mx/en/products/detail/seeed-technology-co-ltd/110990030/5482560) |
| Pines       | 40 pines hembra                 | 2        | 6.7         | [Seeed Studio](https://www.seeedstudio.com/2-54mm-pitch-pin-headers-Female-40pin-in-1-line-p-16.html?indexName=bazaar_retailer_products&objectID=2093&queryID=3d723c52f9b58905e53b48e95182a7c6) |
| Tornillos   | Tornillos M3 x 60 mm            | 4        | 5.0         | [Amazon](https://www.amazon.com.mx/gp/product/B08XB8S55X/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1) |
| Tuercas     | Tuercas hexagonales M3-0.5 mm   | 4        | 1.0         | [Amazon](https://www.amazon.com.mx/gp/product/B07ZFFFRJ5/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1) |
| Popote      | Popote rígido de plástico       | 2        | 3.4         | [Amazon](https://www.amazon.com.mx/Pajitas-Repuesto-Pl%C3%A1stico-Reutilizables-Pinceles/dp/B0925H4F6R/ref=sr_1_18?__mk_es_MX=%C3%85M%C3%85%C5%BD%C3%95%C3%91&crid=IM4WCLYX9S0K&keywords=popote+rigido&qid=1693848668&sprefix=popote+rigido%2Caps%2C150&sr=8-18&ufe=app_do%3Aamzn1.fos.4e545b5e-1d45-498b-8193-a253464ffa471) |
| Cable       | Cable jumper o 22-26 AWG 6 cm   | 2        | 1.8         | [Adafruit](https://www.adafruit.com/product/794) |


# Instrucciones de armado

El proceso de construcción se divide en cinco etapas. La primera etapa describe el ensamblaje del hardware para el monitor de CO2. La segunda etapa detalla la configuración y el procedimiento de carga del firmware para el ESP8266. En la tercera etapa se realiza una prueba de ensamblaje y en la cuarta etapa se lleva a cabo la configuración para la plataforma IoT. La quinta y última etapa implica el ensamblaje de la carcasa y la conexión de la batería USB si es necesario. Es importante destacar que el monitor de CO2 puede construirse y probarse en ausencia de los sensores de CO2, lo que permite adaptarse a posibles demoras en la entrega de componentes en regiones como México.

### Ensamblaje del monitor de CO2

Los siguientes pasos deben llevarse a cabo una vez que el usuario esté listo para construir el monitor de CO2, considerando que se puede probar y configurar aunque no haya un sensor de CO2 en el momento de la construcción, definiendo un sensor ficticio.

1. La PCB es de doble cara y puede producirse mediante serigrafía o mecanizado. Una de las caras tiene impresa la posición de las resistencias, sensores, led, etc., que se llamará cara frontal y el lado sin etiquetar cara trasera. Ambos lados de la PCB se muestran en la Figura~\ref{fig:front_back} a) cara frontal y b) cara trasera.
2. Soldar dos cables puente (6 cm de largo) al botón pulsador.
3. Soldar los siguientes componentes en la cara frontal. Cuatro resistencias, que están marcadas en la PCB como R1 para la resistencia de 10 $kOhm$ y R2, R3 y R4 son resistencias de 220 $Ohm$. Soldar dos líneas de quince pines hembra para el ESP8266. Soldar un conector hembra de cuatro pines para el LED. Para el botón, introducir los cables del botón en esta cara, por lo que la soldadura debería estar en la cara trasera. Insertar el conector vertical grove y soldar. En este punto, la PCB debería parecerse a la Figura~\ref{fig:front_back} c).
4. En la cara trasera, soldar lo siguiente: Soldar el adaptador MicroUSB y el conector hembra de tres pines para el zumbador, el conector hembra para este último debe estar doblado.
5. Si se usa S8LP, soldar dos líneas de conector hembra de seis pines en la cara trasera. En este punto, la cara trasera de la PCB debería parecerse a la Figura~\ref{fig:front_back} d). Luego soldar dos líneas de seis pines al sensor CO2 S8LP y enchufarlo en la PCB como se muestra en la Figura~\ref{fig:co2_sensors} a).
6. Si se usa SCD30 o SEN0220, soldar una línea de conector hembra de cuatro pines en la cara trasera y, dependiendo del sensor, conectar como se muestra en la Figura~\ref{fig:co2_sensors} b) para SCD30 y como se muestra en la Figura~\ref{fig:co2_sensors} c) para SEN0220.
7. Enchufar el cable rojo del LED RGB apuntando al borde de la PCB en la cara frontal.
8. Enchufar la pantalla de 4 dígitos con un cable universal de cuatro pines con hebilla.
9. Enchufar el zumbador en el conector hembra de tres pines doblado.
10. Enchufar el ESP8266 con conexión microUSB alineada con el borde de la PCB.

![Cara frontal de la PCB del monitor de CO2 sin componentes](img/front_pcb.jpg "a) Cara frontal de la PCB sin componentes")
![Cara trasera de la PCB del monitor de CO2 sin componentes](img/back_pcb.jpg "b) Cara trasera de la PCB sin componentes")

![Cara frontal de la PCB del monitor de CO2 con componentes soldados](img/front_soldered.jpg "c) Cara frontal de la PCB con componentes soldados")
![Cara trasera de la PCB del monitor de CO2 con componentes soldados](img/back_soldered.jpg "d) Cara trasera de la PCB con componentes soldados")

**Figura 1**: a) Cara frontal y b) cara trasera de la PCB para el monitor de CO2 sin componentes. Tras soldar los componentes c) cara frontal y d) cara trasera de la PCB. La cara frontal tiene etiquetas para los componentes.




## Autores

En este trabajo es colaboración de (orden alfabético):

* Giovanni Velázquez
* Guadalupe Huelsz
* Guillermo Barrios
* Guillermo Ramirez
* Héctor Daniel Cortés
