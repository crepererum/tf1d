# tf1d - a Traktor F1 userspace driver
This is a userspace driver for Linux that translates the proprietary USB protocol into standard MIDI.

## Requirements
 - ALSA
 - libusb1.0

## Installation
Just make it:

    make build
    sudo make install

To use the F1 without rebooting, you need to reload systemd and udev rules:

    systemctl daemon-reload
    udevadm control --reload-rules

Now plug in (or replug) your F1 and have fun!

## Usage
Plug in your F1 and udev+systemd will start the daemon. Now you can use it as an input device or use ALSA or Jack to route the midi signal. For Bitwig, you can use the [ALSA Virtual MIDI Script](http://www.bitwig.com/en/community/control_scripts/alsa/virtualmidi/virtualmidi_1.html).

## FAQ
Some clarifications.

### Is this something official?
No, this work is not related to Native Instruments or any other music company.

### Why not a proper kernel driver?
It works really well using this userspace solution, so why should I start hacking the kernel? This would also imply difficult debugging and (if I make a mistake) security problems.

### But what about latency?
I am sure you are not able to press the buttons that fast and precisely ;)

### Is there an output mode for lights and stuff?
Not yet. I have a working Python prototype which is able to control all the fancy lights on the device. But the MIDI mapping needs to be done as well as a proper threading model to avoid interference between input and output channel.

### Are the mappings identical to the official Windows/OSX version?
No, but if you provide me a complete list of the mapping, I will change the code to match it.

## Is it possible to use mulitple F1 units?
Currently not. This would require some modification and testing. So if you send a pull request or another F1 unit to me, I will fix that.

