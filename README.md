# Thermal camera using MLX90641 and Wio Terminal

This project is a cheap and simple IR thermal camera using off-the-shelf
components from Seeed Studio:

1. [Wio Terminal](https://www.seeedstudio.com/Wio-Terminal-p-4509.html) (~\$30)
2. [MLX90641 16x12 pixel IR camera as Grove module](https://www.seeedstudio.com/Grove-Thermal-Imaging-Camera-IR-Array-MLX90641-110-degree-p-4612.html) (~\$70)
3. [Grove connector cable](https://www.seeedstudio.com/Grove-Universal-4-Pin-20cm-Unbuckled-Cable-5-PCs-Pack-p-749.html) (~\$3 for 5 cables, you need 1)
4. (Optional) [Wio battery pack](https://www.seeedstudio.com/Wio-Terminal-Chassis-Battery-p-4516.html) (~\$10)

Cheaper camera options are available, but the project currently only supports
this specific one. Others should be relatively easy to add, if you're willing
to do a bit of coding.

I was of course inspired by the page's about the Wio Terminal and the IR
sensor's wiki page, but that version is pretty basic and didn't allow e.g.
live tweaking of emissivity and reflected temperature (and I didn't really
like the code), so I decided to make my own.

## Features

<img src="main_screen.gif" alt="Main screen" width="320" />
<img src="settings_screen.gif" alt="Settings screen" width="320" />

- Display of 16x12 pixel thermal image
- Minimum, maximum and center pixel temperature
- Indication of center pixel
- Legend showing color scale and min/max range used
- Custom emissivity & reflected temperature
- Flip image horizontally / vertically
- Settings window

## Hardware setup

Connect the sensor using the grove cable, on the side of the microphone/speaker,
just as shown on the sensor module's [wiki page](https://wiki.seeedstudio.com/Grove-Thermal-Imaging-Camera-IR-Array/).

Note: for the cable, you need the so-called 'unbuckled' version, i.e. without a
locking catch/lever. If you did order the buckled version (like I did), just cut
if off with a side cutter, otherwise it won't fit in the Wio terminal.

## Software installation

Instructions to get it up-and-running are really easy:

1. Install [VSCode](https://code.visualstudio.com/)
2. Install [Platform IO extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)
3. Clone [this repository](https://github.com/poelstra/thermal-camera)
4. Connect the Wio Terminal through USB
5. In VSCode, press `Ctrl-Shift-P` and choose `PlatformIO: Upload` (or simply press `Ctrl-Alt-U`)

## Usage

The device starts with the camera view and default settings.
You can press the blue joystick's center button to enter the Settings menu.

Navigate around using the joystick, press center to edit a value,
again use the joystick to increase/decrease the value or move which
digit to change. Press center again to leave edit mode.

Navigate to the Close button and press center button to exit the menu.

Testo's [Pocket Guide to Thermography](https://static-int.testo.com/media/1d/b7/21fc65abbea1/Pocket-Guide-Thermography-EN.pdf) appears to be a good source to get acquainted with thermal imaging.
It explains things like emissivity and reflected temperature, and
some advice on how to set/measure them.

## Settings

- **Emissivity**: the ability of a surface to radiate thermal energy.
  It is a value between 0.00 and 1.00. A perfect black body would be
  1.00, a perfect thermal mirror would be 0.00.
  See e.g. [Wikipedia](https://en.wikipedia.org/wiki/Emissivity#Emissivities_of_common_surfaces)
  for typical values to use for certain materials.
  Standard (black) electrical tape has a value of ~0.95.
- **Reflected temperature**: when the emissivity is smaller than 1,
  some of the measured radiation will be from other objects, reflected
  on the subject's surface. Typically, this will be the ambient temperature,
  reflected from e.g. walls and ceiling.
  - **Auto** mode: use the sensor's built-in ambient temperature sensor.
    Note that this sensor measure's the chip's die temperature, so a compensation
    factor is subtracted from it to derive an approximate actual ambient temperature.
    The in-use value is shown in the numeric box. Note that it can take a few
    minutes after start-up for this value to settle.
  - **Custom** mode: when the auto-checkbox is disabled, a value can be set
    using the numeric input field.
- **Flip horizontally**: flip image left-to-right. Useful when moving the sensor to the back.
- **Flip vertically**: flip image top-to-bottom. Useful when moving the sensor to the back.

## Development

### Desktop simulator

The application can be built on the desktop, which simplifies development.
You'll need to have SDL2 installed (including headers) for this to work.

Use the `simulator` target in Platform IO to build it, then choose
`Ctrl-Shift-P` > `Tasks: Run task` > `Run simulator` to run it.

Alternatively, you can get a full debugging experience, but because Platform IO's
native platform [doesn't yet support debugging directly](https://github.com/platformio/platformio-core/issues/980),
you'll need a workaround. Add the following snippet to `.vscode/launch.json` (inside the array):

```json
{
  "name": "Debug simulator",
  "type": "cppdbg",
  "request": "launch",
  "program": "${workspaceFolder}/.pio/build/simulator/program",
  "args": [],
  "environment": [],
  "cwd": "${workspaceFolder}",
  "preLaunchTask": "PlatformIO: Build (simulator)"
}
```

Then, select the `Debug simulator` target in VSCode's Debug view and press `F5`.
Note that this configuration section will be lost after every update to `platformio.ini` etc.

The desktop version uses a 2x zoom to make pixel-perfect tweaking easier.
It can be controlled using (only) the four cursor keys, the `Enter` key, and the `A`,
`B` and `C` keys on your keyboard, to stay close to what is possible on the Wio Terminal.

The IR image shown in the desktop version is a 'recording' of a session using
Wio Terminal. It is currently hardcoded, and doesn't actually respond to e.g. changes
in reflected temperature or emissivity.

### Device debugging

'Real' debugging on the Wio Terminal requires some special provisions, so
'printf-debugging' is probably easiest.

The application has a feature that helps with that: press and hold the joystick's
center button while booting until the blue led (next to the USB port) starts blinking
rapidly (screen will remain off). Then release the button, and use your favorite
terminal program to connect to the serial port (e.g. Platform IO's builtin Serial
Monitor). The boot will continue, including showing any messages during early boot.

## Acknowledgements

- The GUI is built using the excellent [LVGL project](https://lvgl.io/).
- Seeed Studio's Wio Terminal and IR module made this thing possible in the first place,
  including their example projects.
- Melexis provides a convenient library to interface to the device and do the low-level
  thermal calculations.
- Platform IO made getting this thing up and running quite easy, including having both
  a 'real' and simulated version.
- VSCode is a blast.

## License

The application software itself is:

> Copyright (C) 2020 Martin Poelstra, licensed under the MIT license, see LICENSE.md file.

Additionally, the application uses a MLX90641 driver which is licensed as:

> Copyright (C) 2017 Melexis N.V.
>
> Licensed under the Apache License, Version 2.0
