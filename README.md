# ctrbear
Bear for Nintendo 3DS

Bear is the first video game I have ever made. The game is based on a "Game Maker Kids" tutorial, and as such, the first iteration of Bear was a Game Maker game. Since then, Bear has become my "Hello World". I try to recreate Bear in every programming language I learn.

This iteration of Bear is written for the Nintendo 3DS. It is based on the [GameCube/Wii version of Bear](https://github.com/Sevenanths/dolbear), but uses the assets of my 2015 LUA version, also for 3DS. This port was an attempt have a version of Bear 3DS written in C, as I am very new to C (and it probably shows).

![Title screen](https://user-images.githubusercontent.com/6349952/191303228-cd12f4e0-1ca9-490f-900d-7abda1456dfc.png)
![Gameplay](https://user-images.githubusercontent.com/6349952/191303305-0b956295-cb36-421c-a0fc-76467e949c82.png)
![Game over](https://user-images.githubusercontent.com/6349952/191303365-5facd177-b59c-470a-a215-5f11ed4f9854.png)

## Controls

* START: start game/restart game
* Analog stick / D-pad: move bear
* Analog stick / D-pad: select difficulty
* SELECT: exit to homebrew menu

## Compiling

This project hinges on...

- [devkitarm](https://github.com/devkitPro)
- [citro2d](https://github.com/devkitPro/citro2d)
- 3ds-libogg
- 3ds-libvorbisidec ("tremor")

These libraries need to be installed in order to be able to compile Bear.

### Compiling for 3DS

In the ctrbear directory:
```
make
```