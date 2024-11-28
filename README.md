# AlarmClock+
The AlarmClock+ is just like a normal alarm clock, but with a ton of new [features](#features). I would snooze my old alarm clock 4 times before getting out of bed, so I build this new alarm clock. This alarm clock will wake you up and get you out of bed in no time. <br>
This is the first time I built something using an Arduino. The alarm clock consists of electrical components (switches, LEDs and sensors), an Arduino Nano and 3D printed parts. It took around 2-3 months to build the AlarmClock+, and with that came a lot of prototyping, experimenting and head-scratching, but as a result I learned incredibly much.

![Alarm clock Blue](https://user-images.githubusercontent.com/75524368/151206562-98c0101f-f594-488b-8a90-7f907e8b141f.png)

## What have I learned?
But what exactly did I learn in those 2-3 months building the alarm clock? 
- Modeling using SketchUp
- 3D printing
- Prototyping quickly
- Building electric circuits
- Using C++ to program the Arduino

## Endresult

### Features
- <b>Displaying the time</b>
- <b>Changing the colour and brightness of the LEDs</b>
- <b>Changing the time</b>
- <b>Standard Alarm</b><br>
  This alarm works the same as any other alarm. You are woken up at the same time every day.
- <b>Scheduled Alarm</b><br>
  This alarm will wake you up at the time you tell it to on that specific day of the week. For example, you want an alarm at 6:55 on Monday until Friday and no alarm on Saturday and Sunday.
- <b>Getting you out of your bed</b><br>
  8 minutes before the set time, the first alarm will go off. No worries, this alarm can be snoozed by holding your hand above the right sensor. The second alarm goes off at the set time and you will have to get out of bed and grab the RFID tag from your desk. Put the RFID tag on top of the alarm clock and the alarm will stop.

### Video
Watch <b>[THIS VIDEO](https://youtu.be/JE-YsyEofD8)</b> if you would like to see the alarm clock in action.
  
### Photos
![Alarm clock Green](https://user-images.githubusercontent.com/75524368/151209619-bcb4d1e1-d756-4304-893c-af86c8307e68.png)
![Alarm clock Orange](https://user-images.githubusercontent.com/75524368/151210734-b0929efa-ad23-4475-8ff1-183fcc47a87e.png)
![Alarm clock next to bed](https://user-images.githubusercontent.com/75524368/151211021-b248c641-b18e-4783-a39c-82af13f538cc.png)
![Alarm clock spinning](https://user-images.githubusercontent.com/75524368/151211046-be37f432-260c-457e-b855-0324e201b588.gif)

### Parts used
- WS2812B 5050 RGB LED Strip - 30 LEDs 1m
- HC-SR04 Ultrasonic sensor
- RC522 RFID Module
- Rotary Switch 1x11 (x2)
- KY-040 Rotary Encoder
- 3mm CdS photosensitive resistor
- 5v passive buzzer
- DC Jack 5.5mm Female
- Arduino Nano

## File explanation
I included the code in the repository. It could definitely be written better, but it works flawlessly! I also included the .stl files if you would like to 3d print it yourself.

<br>
Ties Petersen [January 2022]
