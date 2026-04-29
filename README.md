# Kangoo can filter

This is repository contains esp32(arduino) version of ElektroBOX can filtering software.

<img width="541" height="794" alt="web interface illustration (ukrainian version)" src="https://github.com/user-attachments/assets/33a05f58-c780-4d94-a413-8594531f1db0" />

The project is not properly documented at the moment. May change in the future.

# TODO
- disable leds if wifi is off

# Testing
(can filter v3)

D19: pulses
- vmax: `4.3`
- vmin: `0.2`

D18: pulses
- vmax: `3.6`
- vmin: `0.2`

D15: pulses
- vmax: `4.13`
- vmin: `0.2`

D14: pulses
- vmax: `3.7`
- vmin: `0.2`

tja1030 left: (top right pin is 1)
pin1: `3.6 pulses : tx`
pin2: `0.0 : gnd`
pin3: `5.3 : vcc`
pin4: `5.2 pulses : rx` (`-> R=1kom -> D19`)
pin5: `3.6 : vref`
pin6: `??? pulses: canl`
pin7: `??? pulses: canh`
pin8: `0.0 : s`

tja1030 right: (down left pin is 1)
...
pin3: `5.3 : vcc`
pin5: `3.6 : vref`
...
pin4: `5.2 pulses : rx` (`-> R=1kom -> D15`)
