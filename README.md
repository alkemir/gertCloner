# gertCloner
Raspberry Pi program to clone HDMI output to GERT666 VGA

We all love the Raspberry Pi, but it lacks VGA output, which is outdated but fitting for MANY of the
crazy ideas you can come up with this wonderful system. Gert van Loo solved this issue when he gave
us the GERT666, a passive board which helps you map the GPIO to a VGA output. It works like a charm
but is has one short-comming: You have to either pick the VGA or the HDMI display. (Except if you
want to play videos using omxplayer https://github.com/huceke/omxplayer
or specially compiled Qt apps: https://www.raspberrypi.org/forums/viewtopic.php?t=135629&p=902277)

Now, with this little program you can clone your screen. It's not as good as two independent screens,
but hopefully this program will get the ball moving.

## Setup

To get the GERT666 running you must download https://github.com/fenlogic/vga666/blob/master/setup/dt-blob-dpi.bin
and move it to ```/boot/dt-blob.bin``` . Then edit the ```/boot/config.txt``` file adding the following lines:

```
dtoverlay=vga666
enable_dpi_lcd=1
display_default_lcd=0
```

The first two lines enable the GERT666, but the last line sets your HDMI display as default.
Finally, disable SPI and I2C interfaces (and I think the Serial Console) through raspi-config and reboot.
To get the same output on your GERT666 you must then run ```./cloner.bin``` .

To get this program on your system just download this repository and ```make```, then run ```./cloner.bin```
and you should get the same image on both screens.

## Command line options
* `-help | -h:` Displays help and flag information.
* `-displaySrc:` ID of the source display. (Default: 0)
* `-displayDst:` ID of the destination display. (Default: 4)
* `-framePeriod:` Delay between frames in milliseconds. (Default: 25)

## How it works

This program works by taking a snapshot from the current output and setting it as the output for the
VGA display. As far as I understand, all is done on the GPU so CPU overhead should be minimal. This
program is inspired by https://github.com/tasanakorn/rpi-fbcp/ . Also, dispmanx.c was of great help
as documentation on the GPU API is... incomplete.

## Contribute

Being honest, my C skillz are non-existant so please please send any changes you would like to publish.
Also, an idea that might work is to configure the framebuffer to be as big as the two screens together,
fiddle a bit with the configuration of this program to copy only the second screen portion and then
we should be able to get the independent dual screen setup! (I think)
