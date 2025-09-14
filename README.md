# ssw-a1000-original

This is my only remaining copy of the original Solar System Wars (SSW) source
code for Amiga 500/1000.  Everything else was lost several moves ago.

## Release history

The last official release was 1.38 on 1995-02-28. (1995-03-03 for AmiNet)

https://aminet.net/package/game/2play/SolarSystemWars

SSW v1.14 was on Fred Fish disk 839.  1993

SSW v1.38 was on Fred Fish disk 1088.

https://ftp.funet.fi/pub/amiga/fish/
https://www.amiga-stuff.com/pd/fish.html

## The Code

What I have is version 1.43, an attempt to make SSW work with the AGA video
chips in higher resolution mode.  I never got it working quite right.

While I've #ifdef-ed out most of the experimental code, I don't think this
version actually compiles.  Nor have I set up an emulation environment to try
and fix that.  Instead, in my spare time (ha!) I'm working to convert SSW to
use SDL2 instead.

https://www.libsdl.org/

I had forgotten just how dependent this SSW was on the A1000 custom chips, so
the conversion is going slowly as I back out all the hacks needed to get 4
stars, 2 ships, 2 extra weapons, and 24 photon torpedos animated with real
gravity calculations at a 60 Hz frame rate using a 7.15 MHz M68000 chip.

As such, I'm releasing what I have as-is under the GPLv3.  May it serve as a
reminder of what we had to do back in the days of bear skins and stone knives.
