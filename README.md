### Intro

I created a wireless low-power eInk display.  Once an image loads onto the eInk display, power can be disconnected and the image will remain. So I created a rechargable battery-powered picture that should last 4 years on one charge!

The eInk display I used is a 10.3 inch 1872×1404 resolution display. Each pixel can display one of 16 shades between black and white.

See this [Youtube Video](https://youtu.be/XQ9fnZ-e4l0) for a demo.

### How my setup works:

 I host a Nextcloud instance on my personal server. Photos are automatcially uploaded from my phone to my photo repository in nextcloud. This project does not expand on  any self-hosting software.

I also host an instance of HomeAssistant and a MQTT server. On my HomeAssistant dashboard there are two images. 1. A random colored image selected from my photo repository. Clicking the image sends a “pull” MQTT command to the MQTT Server. I created a script (runs as service in a Linux container) that listens to certain MQTT requests. Upon hearing the pull request the MQTT Listener will pull a random image from my photo repository and refresh the colored image on HomeAssistant. 2. The other image is the converted grayscale image with the resolution of the eInk display. Clicking the grayscale image sends a “push” requestm taking the “pull” image,  converting it to grayscale, and showing it in HomeAssistant. The image is also converted to a binary format used by the eInk display.

In a Linux container I created a TCP server service. The server waits for the TinyPico to make a connection and then sends the binary grayscale image over TCP.

The TinyPico does not have enough memory to hold the entire image. To get around this I use the ESPIF FreeRTOS (Espressif IoT Development Framework). In this RTOS there is a task/thread that takes the TCP packets and inserts them into a queue. Another task will dequeue the data and write the packet through SPI to the IT8951 driver board. Once the whole image is sent to the IT8951 driver board, TinyPico will send a “display image” command. The ePaper will then refresh with the new image and transition into deep sleep mode. This will cut power to the driver board and display, saving power. TinyPico has two methods of waking up from deep sleep. 1. Timer Interrupt. I set the timer to 24 hours. 2. Capacitive Touch Interrupt.  I soldered the capacitive touch pins to two hinge screws on the bottom of the frame. With these interrupts, the eInk display will update automatically once per day or when someone lightly touches an inconspicuous screw head. Once the display is updated, TinyPico will again enter deep sleep mode.

### Code Location

Once ESPIDF is installed put the eInk folder (found under ESP in this repository)  in the installed esp folder. Inside the esp folder you should see another folder esp-idf.

Look at the “config.h” to change the WiFi credentials

The server code will need to be changed. You can easily change the bash script to change where the images are pulled from.

### Power Calulations

Deep Sleep Mode: 20uA \* 24 h = 480 uAh

Picture Refresh:  1.2 W  for 450 ms

Boost Converter Efficenciy %80

1.2 W / 0.8 = 1.5 Watts

1.5 W → 1.5 W / 3.3 V =  460 mA 

450 ms = 0.000125 hours 

460 mA \* 0.000125 hours = 58 uAh

Picture Load:  250 mA \* 20 s = 1.4 mAh

Total per day:  1. 6 mAh

Battery: 2500 mAh

2500 mAh / 1.6 mAh per day = 1560 days = 4 years

Even by conservative metrics, the battery should last a long time.

### Custom PCB

The custom PCB has the following features/modules: 

*   TinyPico - ESP32’s dual-core 240MHz
*   MiniBoost 5V - boosts the battery voltage to 5V for the IT8951 and eInk display
    *   The tinyPico turns this on or off with a GPIO pin.
*   SPI JST Header to connect to the IT8951 -E-Ink driver board that comes with the ePaper display. 
*   Battery JST connector
*   Circuitry to control the source of the 5V bus is from the USB-C or from the Boost Converter.

Note next time I will only use through-hole parts. I do not have solder oven, so doing it by hand was messy.

### BOM

*   [e-Ink Display](https://www.waveshare.com/product/displays/e-paper/10.3inch-e-paper-hat.htm) 10.3inch 1872×1404, Black / White, 16 Grey Scales, USB / SPI / I80
    *   SKU/ Part Number: 18434
    *   Comes with IT8951 driver board
*   [TINYPICO](https://www.tinypico.com/) (Adafruit PID: 5028)
*   Lithium Ion Polymer Battery - 3.7v 2500mAh (Adafruit PID: 328)
*   MiniBoost 5V @ 1A - TPS61023 (Adafruit PID: 4654)
*   [JST Wires](https://www.amazon.com/gp/product/B096QG96J4/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) for SPI 
*   SPI Header - B8B-PH-SM4-TB 
*   JST Battery Header - B2B-PH-SM4-TBT
*   MOSFET P-CH - AOSS21319C
*   Diode - B5819WS-TP
*   Resistors
*   Shadow Box Frame [Amazon](https://www.amazon.com/gp/product/B082PZ11Y1/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)

### Home Assistant

In configuration.yaml:

```text-plain
mqtt:
  camera:
    - topic: homeassistant/camera/pull
    - topic: homeassistant/camera/push
```

My HomeAssistant YAML grid card configuration:

```text-plain
square: true
columns: 2
type: grid
cards:
  - type: picture-entity
    show_state: false
    show_name: true
    camera_view: auto
    entity: camera.mqtt_camera
    name: Pull
    tap_action:
      action: call-service
      service: mqtt.publish
      data:
        topic: homeassistant/eink/pull
        payload: pull
      target: {}
  - type: picture-entity
    show_state: false
    show_name: true
    camera_view: auto
    entity: camera.mqtt_camera_2
    name: Push
    tap_action:
      action: call-service
      service: mqtt.publish
      data:
        topic: homeassistant/eink/push
        payload: push
      target: {}
```

My HomeAssistant YAML Automation Timer - (Resets the Picture once a day):

```text-plain
alias: eInk
description: ""
trigger:
  - platform: time
    at: "00:00:00"
  - platform: time_pattern
    minutes: /5
    enabled: false
condition: []
action:
  - service: mqtt.publish
    data:
      topic: homeassistant/eink/pull
      payload: pull
      qos: 0
      retain: false
  - delay:
      hours: 0
      minutes: 0
      seconds: 10
      milliseconds: 0
  - service: mqtt.publish
    data:
      topic: homeassistant/eink/push
      payload: push
      qos: 0
      retain: false
mode: single
```

### Image Conversion

The images are converted primarily using ImageMagick command line:

```text-plain
$ sudo apt install imagemagick
```

This command converts the image to a colored jpg with the correct dimensions:

```text-plain
convert "$PIC" -auto-level -resize 1872x1404 -background black -gravity center -extent 1872x1404 resize.jpg
```

This command converts the image to grayscale jpg:

```text-plain
convert "$PIC" -grayscale Rec709Luminance -depth 4 -linear-stretch 6.25x6.25% -resize 1872x1404 -background black -gravity center -extent 1872x1404 gray.jpg
```

This command converts the image to a grayscale of 4 bits pgm image.

```text-plain
convert "$PIC" -grayscale Rec709Luminance -depth 4 -linear-stretch 6.25x6.25% -resize 1872x1404 -background black -gravity center -extent 1872x1404 -flop pic.pgm
```

Note: you may want to play with the contrast.

Because the display has 16 shades per pixel, each pixel can be represented as 4 bits/ a nibble. When converting an image to pgm format, each pixel is represented by a byte. I wrote a C program that takes a pgm format (see Resources), strips the header and compresses the data so that a byte represents two pixels. This compresses the memory size in half. This compressed file is sent using the TCP server. See pgmCompress.c

### TCP Server

The TCP Server is based off this guide: [https://www.beej.us/guide/bgnet/html/#client-server-background](https://www.beej.us/guide/bgnet/html/#client-server-background).

When a connection is made a child process is created and the once the image is sent the child process ends.

### Shadow Box Frame

I ordered a shadow box frame from amazon and then removed the glass and sanded the inside. I then hot glue the eink display to the inside. This was more effort then it was worth and you want it to look nice. I would recommend you have your eInk display framed by a professional. 

### Resources

[eInk Display Waveshare](https://www.waveshare.com/wiki/10.3inch_e-Paper_HAT)

[TinyPICO](https://www.tinypico.com/)

[MQTT Server](https://mqtt.org/)

[MQTT Camera - Home Assistant](https://www.home-assistant.io/integrations/camera.mqtt/)

[PGM Image File Format](https://users.wpi.edu/~cfurlong/me-593n/pgmimage.html) 

[ESP-IDF Install](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/)

[ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/index.html)

[ImageMagick – Convert, Edit, or Compose Digital Images](https://imagemagick.org/index.php)

[TCP Server](https://www.beej.us/guide/bgnet/html/#client-server-background)