## EleksTubeHAX - An aftermarket custom firmware for the "EleksTube IPS clock" and "SI HAI IPS clock" and "Novellife SE clock"

Buy your own clock under the name "EleksTube IPS Clock" on eBay, Banggood, etc.
Look for places that offer 30-day guarantee, otherwise it can be a fake product!

[Reddit discussion on the original hack is here.](https://www.reddit.com/r/arduino/comments/mq5td9/hacking_the_elekstube_ips_clock_anyone_tried_it/)

[Original documentation and software from EleksMaker.](https://wiki.eleksmaker.com/doku.php?id=ips)

Firmware supports and was tested on those three clock versions:
![EleksTube IPS clock](/Photos/EleksTube_original_PCB.jpg)
![SI HAI IPS clock](/Photos/SI_HAI_ips_clock.jpg)
![Novellive SE clock](/Photos/NovellifeSE.jpg)

## Updates from aly-fly's and Smitty-Halibut's version:

## Additional features/changes/fixes:

- Only do NTP call on boot and on Sundays.  I don't want to beat up the NTP servers, but checking on Sunday will get the Daylight Savings clock change.
- Change the UI so that left button causes the display to show the date for 3 seconds.  Pushing the right button jumps directly to the menu item for changing the font.  Center (mode) button is unchanged, still enters the menu at the top.  "Power" button still exits the menu.
- Allow wifi credentials to be entered using the serial interface. I did this because my router doesn't do WPS.
- Allow empty fonts to exist; the code thinks there are always 10 fonts installed, but it'll just display blanks if the font files are missing.  This means that you can have fonts 1,3,5 installed and the clock still allows you to choose any of the installed fonts.  (Before, a missing font meant that you couldn't use higher numbered fonts after the first missing font.)
- Allow NIGHT_TIME to be midnight (0) or early AM and still have the display dim between NIGHT_TIME and DAY_TIME.  (Before, NIGHT_TIME had to be larger than DAY_TIME, which procluded hours 0,1,2 . . .)
- organized a bunch of fonts under the "all_fonts" directory.
- trimmed some of the fonts (removed black borders) to shrink font size.  The code will automatically center the fonts (aly-fly feature).
- Wrote a Perl script to allow easier font management.  It allows you to copy a set of font files from the "all_fonts/font-name" directory to the "data" directory.
- Wrote a Perl script to turn the raw binary fonts into ".CLK" files.
- Wrote a Perl script to turn the BMP font files into PNG and ".CLK" files.
- Wrote a Perl script to turn the ".CLK" files into PNG and BMP files.
- Added instructions for creating a "better" partitioning scheme for the
flash on the ESP32 so that you can get more fonts in there.
I can fit two large fonts and three small fonts in the revised partition.

## How to create a "better" partition scheme (more SPIFFS!)

This code won't quite fit in a 1M/3M partition setup, so you need to create
a new 1.1M/2.9M partition scheme.
Use this reference: https://robotzero.one/arduino-ide-partitions/

I created a new partition scheme:

`Name,     Type, SubType, Offset,  Size,     Flags`
`nvs,      data, nvs,     0x9000,  0x5000,`
`otadata,  data, ota,     0xe000,  0x2000,`
`app0,     app,  ota_0,   0x10000, 0x120000,`
`spiffs,   data, spiffs,  0x130000,0x2D0000,`


Which has the spiffs partition turned up to the maximum space available.
I called it noota_2g.csv

Then, open up the boards.txt file, and find the section that list the
partitions available for that board.  I chose (somewhat randomly)
the "ESP32 Wrover Module".

I copied the three lines that start:

`esp32wrover.menu.PartitionScheme.noota_3g`

and pasted and edited the copied lines to look like this:

`esp32wrover.menu.PartitionScheme.noota_2g=No OTA (1.2MB APP/2.9MB SPIFFS)
esp32wrover.menu.PartitionScheme.noota_2g.build.partitions=noota_2g
esp32wrover.menu.PartitionScheme.noota_2g.upload.maximum_size=1179648`

You will note that the second line references the csv file we created
in the first step.

Now save boards.txt with the three new lines installed.

(This assumes you have "ESP32 Wrover Module" selected as your "Board".
If not, adapt these instructions for the Board you have chosen.)

Now restart your arduino IDE, and you will find your new partition
scheme available in the menu.

See [aly-fly's readme](/README_aly-fly.md) for more.
See [SmittyHalibut's readme](/README_SmittyHalibut.md) for more.

# Contributors
- Mark Smith, aka Smitty ... @SmittyHalibut on GitHub, Twitter, and YouTube.
- Aljaz Ogrin, aka aly-fly ... @aly-fly on GitHub and Instagram
- William Dudley, @wfdudley on GitHub
