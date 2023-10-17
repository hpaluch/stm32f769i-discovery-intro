# STM32F769I Discovery Intro

Here I plan to make set of projects for [32F769IDISCOVERY Discovery kit][32F769IDISCOVERY]
with the final goal being able to use LCD, Audio Codec and accessories (SD card, external Flash, SDRAM and others)

If you are new to [32F769IDISCOVERY Discovery kit][32F769IDISCOVERY] I suggest
reading [Getting started with STM 32F769IDISCOVERY][GS32F769IDISCOVERY] first.

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

# Projects

Common project properties:
- LED `LD1_USER` RED will be on when `Error_Handler()` in `main.c` is called,
  which means "fatal error, system halted"

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

Planned projects:
2. UART1 Tutorial - redirect `printf(3)` to UART1 which is connected to Virtual COM port of ST-LINK.
   Rationale: UART is perfect tool for diagnostics and error messages.
3. LCD Display simple demo. LCD is perfect for feedback and interactivity
4. Audio setup: will be used for demos
And more...

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

[STM32f769NI]: https://www.st.com/resource/en/datasheet/stm32f769ni.pdf
[UM2033]: https://www.st.com/resource/en/user_manual/um2033-discovery-kit-with-stm32f769ni-mcu-stmicroelectronics.pdf 
[GS32F769IDISCOVERY]: https://github.com/hpaluch/hpaluch.github.io/wiki/Getting-started-with-32F769IDISCOVERY
[32F769IDISCOVERY]: https://www.st.com/en/evaluation-tools/32f769idiscovery.html
[RM0410]: https://www.st.com/resource/en/reference_manual/rm0410-stm32f76xxx-and-stm32f77xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf
[STM32CubeIDE-Win]: https://www.st.com/en/development-tools/stm32cubeide.html
[STM32CubeF7]: https://www.st.com/en/embedded-software/stm32cubef7.html
[STM32CubeMX]: https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-configurators-and-code-generators/stm32cubemx.html
[STM32CubeProg]: https://www.st.com/en/development-tools/stm32cubeprog.html

