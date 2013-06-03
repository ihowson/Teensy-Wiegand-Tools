# Teensy Wiegand dumper

Ian Howson (ian@mutexlabs.com)

This is a modification of the [Teensy USB keyboard emulation code](http://www.pjrc.com/teensy/usb_keyboard.html) to dump out Wiegand strings. For example, you can connect an off-the-shelf HID card reader and dump out card contents.

The intended hardware setup is:

- Teensy 2.0 connected to PC over USB
- Reader 5V and GND connect to the Teensy
- Reader D0 connects to PB0
- Reader D1 connects to PB1

If you swipe a card, the Teensy should type out the card's raw code on your PC.

The code is extremely nasty and needs work. Also, I haven't provided a clear line between PJRC's code and mine. 

Assume that it is all MIT licensed.

This is intended to support [Attacks on Proximity Card Systems](http://ianhowson.com/attacks-on-proximity-card-systems.html)


