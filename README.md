# 7-Segment FastClock / Counter Display

This is a controller for a LED driven 7-segment clock/counter display. An ESP8266 offers WiFi access for configuration/setting purpose and the ability to retrieve the current time through NTP.

But it might as well be used as a fast clock display for model railroads.

Prerequisites:

-   WS-2812 based adressable LED chain forming the 7-segment display
-   number of LEDs per segment is configurable

The order of the WS2812 LEDs is important. The library could be enhanced to get more flexibility here, on the other hand there seems to be not that many useful options. For now, please follow the following order of the LEDs for each digit:

```
   -3-
 4|   |2
   -1-
 5|   |7
   -6-
```

To display times like 12:34 or temperatures like 12.3° we have 2 digits, a seperator (2 dots) and a decimal point followed by another 2 digits. Thus all WS2812 LEDs are on a single chain for the complete display.

This is, how it could look like (fyi: the big cargo car is 1:45 scale, the small one is 1:160):

![Clock Picture](doc/Clock1a.png)

The configuration menu allows the selection of real or fast clock and some options like colors, fastclock name or UTC time offset for real time:

![Config Menu](doc/WebConfig.png)

## Links / References

-   example of 3d printable segment frames: http://www.thingiverse.com/thing:2947883
