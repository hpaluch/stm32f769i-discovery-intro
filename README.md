# STM32F769I Discovery Intro

Here I plan to make set of projects for [32F769IDISCOVERY Discovery kit][32F769IDISCOVERY]
with the final goal being able to use LCD, Audio Codec and accessories (SD card, external Flash, SDRAM and others)

If you are new to [32F769IDISCOVERY Discovery kit][32F769IDISCOVERY] I suggest
reading [Getting started with STM 32F769IDISCOVERY][GS32F769IDISCOVERY] first.

CubeMX vs LCD Examples:

Unfortunately just found that existing LCD Examples are not supported by CubeMX as shown below:

![LCD Examples incompatible with CubeMX](https://github.com/hpaluch/hpaluch.github.io/wiki/files/stm32/cubemx-lcd-ex-incompat.jpg)

Details can be found on [My Wiki page](https://github.com/hpaluch/hpaluch.github.io/wiki/Getting-started-with-32F769IDISCOVERY#cubemx-warning).

It means that I have to choice one of:

1. clone example code, but add/or modify all hardware components manually
2. use CubeMX but duplicate BSP code for LCD and possibly other components (LCD, Audio,...)

Since project #3 - [Disco3_LCD/](Disco3_LCD) I'm using middle way approach:
- I enable all used peripherals in CubeMX - that is required for Clock configuration
  and other stuff
- However for initialization and API I use BSP routines. You can see it
  in CubeMX -> Project Manager -> Advanced Settings where I simply
  unchecked `Generate Code` for all hardware that is initialized by BSP.
- BSP headers and modules are included using [Disco3_LCD/.extSettings](Disco3_LCD/.extSettings)

Here is hardware fully managed by CubeMX:
- GPIO LEDs, Buttons, MPU
Hardware managed by BSP functions:
- SDRAM
- DMA1 (configured by BSP SDRAM routines)

# Requirements

Required hardware:
* [32F769IDISCOVERY Discovery kit][32F769IDISCOVERY] 
  - tested mainboard: `MB1225-F769I-B03` 
  - tested LCD daughterboard: `MB1166-DEFAULT-A09`
* USB A to Micro-B cable for Programming board and for Power (Power switch in `stlink` mode tested)
  (cable is not included with board)
* for some project MicroSD card needed (will be refined later)

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
  you will have issues when building my projects under  STM32CubeIDE.

WARNING! For Disco3_LCD and more recent projects I use relative
Firemware folders where possible. You therefore need to
copy content of `c:\Ac6\STM32Cube\Repo\STM32Cube_FW_F7_V1.17.1\Drivers`
to parent directory `..\`. So there will exist folder `..\Drivers`

WARNING! Despite my best efforts the CubeIDE file [Disco3_LCD/.cproject](Disco3_LCD/.cproject)
still containts absolute include paths, although the [Disco1_GPIO/Disco1_GPIO.ioc](Disco1_GPIO/Disco1_GPIO.ioc) uses properly relative firmware path. I think that it is bug
in CubeMX.

# Projects

Common project properties:
- LED `LD1_USER` RED will be on when `Error_Handler()` in `main.c` is called,
  which means "fatal error, system halted"
- for project No. 2 or later there is also active Debug UART output. To see it, connect
  Putty (or other serial terminal) to
  `STMicroelectronics STLink Virtual COM Port (COMx)` - notice `x` in `COMx` and
  use: Speed: 115200 baud, 8 data bits, no-parity, no flow control.

Here is list of projects:

Finished projects:

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

Projects in progress:

3. Requierd peripherals for LCD and Audio:
   - I2C4 (both Audio and TouchScreen - TS use I2C4 for commands)

# Configuring SDRAM

Needed as graphics RAM for LCD and other stuff. SDRAM has impressive 16MB -
frequency is limited to 200 MHz (CPU is capable of 216 MHz).

SDRAM needs:
- FMC (Flexible Memory Controller)
- MPU (Memory protection Unit) - to configure SDRAM region and FMC registers

SDRAM requires following CubeMX components
- System Core -> `CORTEX_M7` -> MPU - defines SDRAM region and FMC region
- Connectivity -> `FMC` -> `SDRAM 1`
However again - I unchecked `Generate Code` in CubeMX and I'm using
BSP methods to manage SDRAM and FMC.

Numbers:
- Write of 16 MB SDRAM (Caches disabled) takes around 4s.
  Read is just a bit faster.

# Configuring LCD

It is probably most difficult peripheral on board (2nd is Audio codec).

However just found two important application notes:
* [AN4860: Introduction to DSI host on STM32 MCUs and MPUs][AN4860]
* [AN4861: LCD-TFT display controller (LTDC) on STM32 MCUs][AN4861]

Especially [AN4860][AN4860] contains important chapter:
- `6  STM32CubeMX configuration example`
- that is for - quoting:

  > The examples have been generated for the STM32F769I-Discovery board.

- however please note that above example is for older `OTM8009` chip.
- so in my case I have to carefully evaluate changes needed for 
  my `A09` display board revision with `NT35510` chip.
- but it is without doubt important help!


We need to enable these CubeMX peripherals:
- Multimedia -> DMA2D (not required but BSP uses it)
  - NOTE! DMA2D is different from DMA!
- DSI PLL ititialization
- DSI ititialization
- LTDC ititialization
- ~~OTM8009A~~  NT35510 LCD Display IC Driver via I2C

However I will use BSP routines to initialize these...

# Planned projects


Planned projects:

3. LCD Display simple demo. LCD is perfect for feedback and interactivity
4. Audio setup: will be used for demos

But it will be challenging to use CubeMX for it, because existing LCD examples
are not compatible with CubeMX and they use BSP routines that often clashes with CubeMX.

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

