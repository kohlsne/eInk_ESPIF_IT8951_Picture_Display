#!/bin/bash
X_EINK=1872
Y_EINK=1404
MQTT_HOST=172.31.0.106
MQTT_USER=USER
MQTT_PASSWORD=PASSWORD
MQTT_PORT=1883
PIC="/pic.jpg"

gcc -o main pgmCompress.c
mosquitto_sub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASSWORD -t homeassistant/eink/\# | while read -r payload
do
	echo "Rx MQTT: ${payload}"
	if [[ $payload == *"pull"* ]]; then
		echo "pull"
		PIC=$(find /mnt/media/eInk_Photos -name '*' |sort -R |tail -1)
		convert "$PIC" -auto-level -resize 1872x1404 -background black -gravity center -extent 1872x1404 resize.jpg
		mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASSWORD -t homeassistant/camera/pull -f resize.jpg
	elif [[ $payload == *"push"* ]]; then
		echo "push"
                convert "$PIC" -grayscale Rec709Luminance -depth 4 -linear-stretch 6.25x6.25% -resize 1872x1404 -background black -gravity center -extent 1872x1404 gray.jpg	
		mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASSWORD -t homeassistant/camera/push -f gray.jpg
	        convert "$PIC" -auto-level -grayscale Rec709Luminance -depth 4 -linear-stretch 6.25x6.25% -resize 1872x1404 -background black -gravity center -extent 1872x1404 -flop pic.pgm
		echo "Parsing Started/C code"
		./main
		echo "Parsing Complete/C code"
	fi
done

