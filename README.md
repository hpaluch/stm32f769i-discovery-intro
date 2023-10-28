# STM32F769I Discovery Intro

Here are examples how to utilize both [CubeMX][STM32CubeMX] and BSP libraries for
[32F769IDISCOVERY Discovery kit][32F769IDISCOVERY] to be able to use LCD
display and other board peripherals with least amount of work.

Below is image of 3rd project [Disco3_LCD/](Disco3_LCD/) - which is 1st one
that uses LCD display:

![Disco3_LCD example](assets/disco3_lcd_ex.jpg)

It is not as easy as you may expect because official firmware examples
are NOT [CubeMX][STM32CubeMX] compatible as can be seen below:

![LCD Examples incompatible with CubeMX](https://github.com/hpaluch/hpaluch.github.io/wiki/files/stm32/cubemx-lcd-ex-incompat.jpg)

However I have found relatively simple workaround:
1. Use [CubeMX][STM32CubeMX] to **enable** all needed peripherals (required to enable
   configuration of Clocks).
1. Set "Do not Generate Code" for peripherals that will
   be **initialized** and **managed** using BSP library.

NOTE: If you are new to [32F769IDISCOVERY Discovery kit][32F769IDISCOVERY] I suggest
reading [Getting started with STM 32F769IDISCOVERY][GS32F769IDISCOVERY] first.

# Requirements

Required hardware:
* [32F769IDISCOVERY Discovery kit][32F769IDISCOVERY] 
  - tested mainboard: `MB1225-F769I-B03` 
  - tested LCD daughterboard: `MB1166-DEFAULT-A09`
* USB A to Micro-B cable for Programming board and for Power (Power switch in `stlink` mode tested)
  (cable is not included with board)

Required software:
* [STM32CubeMX][STM32CubeMX] tested v6.9.2 - code generator - all projects here are generated with this tool
* [STM32CubeIDE-Win][STM32CubeIDE-Win] teste v1.13.2 - development IDE
* [STM32CubeProg][STM32CubeProg] - for some projects `STM32 Cube Programmer` will be required
  to write data to external Flash (CubeIDE is not able to do this)

NOTE: When you run STM32CubeMX for the first time you have to:
- select Help -> Managed embedded software packages
- select tab `STM32Cube MCU Packages`
- expand `STM32F7`
- select and install `STM32Cube MCU Package for STM32F7 Series` version 1.17.1
- try to use target folder `c:\Ac6\STM32Cube\Repo\STM32Cube_FW_F7_V1.17.1` otherwise
  you will have to regenerate STM32CubeIDE files from CubeMX to update
  absolute paths in CubeIDE files (they are always absolute)

IMPORTANT!

Starting with `Disco3_LCD` examples you need to copy portion
of STM32F7 firmware to **parent** directory of this project:
- if you have firmware in directory `c:\Ac6\STM32Cube\Repo\STM32Cube_FW_F7_V1.17.1\`
- you need to copy its `\Drivers` and `\Utilities` directory
  to parent directory of this project so `..\Drivers` and `..\Utilities`
  are available

Although CubeMX files `*.ioc` and `.extSettings` use only relative
paths, the generated CubeIDE files - `.cproject` still contain
**Absolute pathnames**.  So this is valid:

* If your firmware is in other directory than `c:\Ac6\STM32Cube\Repo\STM32Cube_FW_F7_V1.17.1\` you will need to regenerated CubeIDE files from CubeMX
  for `Disco1_GPIO/` and `Disco2_UART/` projects.
* If you checked out this project to different directory from `c:\projects\STM32F7\stm32f769i-discovery-intro\` you will need to use CubeMX to regenerate
  CubeIDE files for `Disco3_LCD` project.

# Projects

Common project properties:
- LED `LD1_USER` RED will be on when `Error_Handler()` in `main.c` is called,
  which means "fatal error, system halted"
- for project No. 2 or later there is also active Debug UART output. To see it, connect
  Putty (or other serial terminal) to
  `STMicroelectronics STLink Virtual COM Port (COMx)` - notice `x` in `COMx` and
  use: Speed: 115200 baud, 8 data bits, no-parity, no flow control.

Here is list of projects:

## Finished: 1. GPIO LEDs and Buttons

1. GPIO Tutorial - LEDs and Switch.
   Rationale: LEDs provide early feedback is system is alive and when something
   goes wrong.
   - What it does:
     - blinking `LD_USER2 Green LED` and inverted blinking `LD3 Green LED` on PA12
     - when you push Blue user button both Green LEDs will blink faster as long as button is held.
   - measured consumption: 4.89V, 0.25A => 1.22W
   - used hardware:
     - `LD_USER1` (LD1) red LED on PJ13
     - `LD_USER2` (LD2) green LED on PJ5
     - LD3 green LED on PA12 (also used for used for Arduino as `SCK/D13`)
     -`B_USER` "User & WakeUp Button" on PA0

## Finished: 2. UART + printf

2. UART1 Tutorial - redirect `printf(3)` to UART1 which is connected to Virtual COM port of ST-LINK.
   Rationale: UART is perfect tool for diagnostics and error messages.
   - What is does:
     - blinking `LD_USER2 Green LED` at 200ms rate when Blue button is not pressed
     - blinking `LD3 Green LED on PA12` at 200ms rate when Blue button is pressed
     - printing debug message on UART every 1 second like this:
       ```
       L105: App init v1.00
       L115: #0 Ticks=2
       L115: #10 Ticks=1014
       L115: #20 Ticks=2026
       ...
       ```
     - where `Lx` is line number, `#x` is loop iteration (UART output on every 10th 
       to not overflow serial terminal). `Ticks` are Ticks returned from `HAL_GetTick()`
       function which uses 1 tick per 1ms.
  - measured consumption: U=4.88 [V], I=0.33 [A], `P=U*I`=1.61 [W]
  - I suspect that such current increase (compared to 0.25A on GPIO example) is because
    UART RX uses 16x oversampling (so clock is roughly `115200*16 = 1.8 MHz` - so it
    requires high-speed GPIO. But I'm not sure about this.
  - WARNING! It may be tempting to enable Pull-Up on `PA10` which is `VCP_RX` (Virtual COM
    port RX) to avoid floating Input. However I'm NOT recommending this, because
    PA10 and PB12 pins are two pins with strong pull-ups (10 kOhm instead of 40 kOhm
    when compared to other GPIO Pins - required when configured for USB).
    Thus I rather left default - `VCP_RX` without Pull-Up

## Working: 3. LCD

LCD project (`Disco3_LCD`) now works. However I plan to 
tune some details.

WARNING! Works only for NT35510 (Display daughterboard revision A09 or later)

Recommended for study:
* [AN4860: Introduction to DSI host on STM32 MCUs and MPUs][AN4860]
* [AN4861: LCD-TFT display controller (LTDC) on STM32 MCUs][AN4861]

# Future

Planned projects:

4. Audio setup: will be used for demos

# Notes

Because I plan to use external SDRAM in future projects - I'm limited to 200 MHz max CPU frequency.
Here is quote:
```
// c:\Ac6\STM32Cube\Repo\STM32Cube_FW_F7_V1.17.1\Projects\STM32F769I-Discovery\Examples\BSP\readme.txt

@note The STM32F7xx devices can reach a maximum clock frequency of 216MHz but as this example uses SDRAM,
      the system clock is limited to 200MHz. Indeed proper functioning of the SDRAM is only guaranteed
      at a maximum system clock frequency of 200MHz.
```

Here is recommended setup for STM32CubeF7 firmware:
```c
// c:\Ac6\STM32Cube\Repo\STM32Cube_FW_F7_V1.17.1\Projects\STM32F769I-Discovery\Examples\BSP\Src\main.c

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 200000000
  *            HCLK(Hz)                       = 200000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 400
  *            PLL_P                          = 2
  *            PLL_Q                          = 8
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 6
  * @param  None
  * @retval None
  */
```

As noted above we use external X2 High Speed Oscillator (HSE)  at 25 MHz.

# Resources

* Please see my [Getting started with STM 32F769IDISCOVERY][GS32F769IDISCOVERY] 
  for introduction.
* [UM2033 Discovery kit with STM32F769NI MCU][UM2033] essential DISCOVERY board manual.
* [STM32f769NI Datasheet][STM32f769NI] Datasheet
* [RM0410][RM0410] - ARM programmers manual

[AN4860]: https://www.st.com/content/ccc/resource/technical/document/application_note/group0/c0/ef/15/38/d1/d6/49/88/DM00373474/files/DM00373474.pdf/jcr:content/translations/en.DM00373474.pdf
[AN4861]: https://www.st.com/content/ccc/resource/technical/document/application_note/group0/25/ca/f9/b4/ae/fc/4e/1e/DM00287603/files/DM00287603.pdf/jcr:content/translations/en.DM00287603.pdf
[STM32f769NI]: https://www.st.com/resource/en/datasheet/stm32f769ni.pdf
[UM2033]: https://www.st.com/resource/en/user_manual/um2033-discovery-kit-with-stm32f769ni-mcu-stmicroelectronics.pdf 
[GS32F769IDISCOVERY]: https://github.com/hpaluch/hpaluch.github.io/wiki/Getting-started-with-32F769IDISCOVERY
[32F769IDISCOVERY]: https://www.st.com/en/evaluation-tools/32f769idiscovery.html
[RM0410]: https://www.st.com/resource/en/reference_manual/rm0410-stm32f76xxx-and-stm32f77xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf
[STM32CubeIDE-Win]: https://www.st.com/en/development-tools/stm32cubeide.html
[STM32CubeF7]: https://www.st.com/en/embedded-software/stm32cubef7.html
[STM32CubeMX]: https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-configurators-and-code-generators/stm32cubemx.html
[STM32CubeProg]: https://www.st.com/en/development-tools/stm32cubeprog.html

