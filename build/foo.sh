#!/bin/bash

while true
do 

  while ! test -e /dev/ttyUSB0
  do
	  echo 'Connect ESP8266...'
	  sleep 2
  done

esptool.py --port=/dev/ttyUSB0 --baud=460800 write_flash 0x0 monitorCO2.ino.bin

  while test -e /dev/ttyUSB0
  do
	  echo 'Disconnect ESP8266...'
	  sleep 2
  done

  sleep 2
done
