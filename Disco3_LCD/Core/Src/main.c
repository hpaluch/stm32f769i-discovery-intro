/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h> // rand(3), srand(3)...
#include <stdio.h> // printf(3) redirected to UART1, ST-Link Virtual COM port
#include <string.h> // snprintf(3)
#include <inttypes.h> // printf(3) format macros for stdtypes.h, for example PRIu32 to print uint32_t
#include <stdbool.h> // bool type and true/false
#include <machine/endian.h> // __bswap16(), __htons(), __ntohs()
// BSP includes
#include "stm32f769i_discovery.h"
#include "stm32f769i_discovery_sdram.h"
#include "stm32f769i_discovery_lcd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define APP_VERSION 106 // 123=1.23
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static const char *APP_NAME="Disco3_LCD";
// true if it is possible to write to UART
bool gUartStarted=false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Initialize(void);
static void MPU_Config(void);
// from: STM32Cube_FW_F7_V1.17.1\Projects\STM32F769I-Discovery\Examples\ADC\ADC_TemperatureSensor\Src\main.c
static void LCD_Config(void);
/* USER CODE BEGIN PFP */
// Redirecting printf(3) to UART1 that is connected to ST-Link Virtual COM port
// from: STM32Cube_FW_F7_V1.17.1\Projects\STM32F767ZI-Nucleo\Examples\UART\UART_Printf\Src\main.c
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  unsigned loopCount = 0;
  char buf[128] = {0,};
  int oldBufLen=0,newBufLen;
  sFONT *pFontInfo;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  gUartStarted=true; // tell Error_Handler() that it can dump error on UART
  // enable LD3 Green on PA12
  HAL_GPIO_WritePin(LD3_GREEN_GPIO_Port, LD3_GREEN_Pin, GPIO_PIN_RESET);
  printf("\r\nL%d: %s init v%d.%02d\r\n", __LINE__, APP_NAME, APP_VERSION/100, APP_VERSION%100);

  // BSP function that initializes SDRAM & DMA1
  if (BSP_SDRAM_Init()!=SDRAM_OK){
	  printf("ERROR: L%d BSP_SDRAM_Init() failed.\r\n",__LINE__);
	  Error_Handler();
  }

  // initialize LCD using BSP and draw static texts
  LCD_Config();
  pFontInfo = BSP_LCD_GetFont();
  if (pFontInfo == NULL){
	  printf("ERROR: L%d BSP_LCD_GetFont() returned NULL!\r\n",__LINE__);
	  Error_Handler();
  }
  printf("L%d: Configured LCD Display WxH = %" PRIu32 "x%" PRIu32
		 " Font HxW = %" PRIu16 "x%" PRIu16 "\r\n",
		  __LINE__,BSP_LCD_GetXSize(), BSP_LCD_GetYSize(),
		  pFontInfo->Height, pFontInfo->Width);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	// try to not overflow UART output - once per second is enough.
	if (loopCount%10 == 0){
	    snprintf(buf,sizeof(buf),"#%u HAL_Ticks=%" PRIu32, loopCount, HAL_GetTick());

	    newBufLen = strlen(buf);
	    if (newBufLen < oldBufLen){
		    // These 3 commands below are required only when new text is shorter than old text.
		    // They will clear full width of text line
		    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		    BSP_LCD_FillRect(0,BSP_LCD_GetYSize()/2 + 45, BSP_LCD_GetXSize(), pFontInfo->Height);
		    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	    }
	    oldBufLen = newBufLen;

	    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()/2 + 45, (uint8_t *)buf, CENTER_MODE);

		printf("L%d: #%u HAL_Ticks=%" PRIu32 "\r\n",
				 __LINE__,loopCount,HAL_GetTick());
	}
	++loopCount;

	if (HAL_GPIO_ReadPin(B_USER_GPIO_Port, B_USER_Pin)== GPIO_PIN_SET){
		// when blue user button is pressed - blink LD3 LED
		HAL_GPIO_TogglePin(LD3_GREEN_GPIO_Port, LD3_GREEN_Pin);
		HAL_GPIO_WritePin(LD_USER2_GREEN_GPIO_Port, LD_USER2_GREEN_Pin, GPIO_PIN_RESET);
		// uncomment this if you want to test Fatal error handler on Button Push: Error_Handler();
	} else {
		// default - blink LD1 LED
		HAL_GPIO_TogglePin(LD_USER2_GREEN_GPIO_Port, LD_USER2_GREEN_Pin);
		HAL_GPIO_WritePin(LD3_GREEN_GPIO_Port, LD3_GREEN_Pin, GPIO_PIN_RESET);
	}
    HAL_Delay(100); // wait 0.1s
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
// Redirecting printf(3) to UART1 that is connected to ST-Link Virtual COM port
// from: STM32Cube_FW_F7_V1.17.0\Projects\STM32F767ZI-Nucleo\Examples\UART\UART_Printf\Src\main.c
/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART3 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}

// from STM32Cube_FW_F7_V1.17.1\Projects\STM32F769I-Discovery\Examples\ADC\ADC_TemperatureSensor\Src\main.c
/**
  * @brief  Configure the LCD for display.
  * @param  None
  * @retval None
  */
static void LCD_Config(void)
{
  uint32_t  lcd_status = LCD_OK;
  char title[64] = {0,};

  /* Initialize the LCD */
  lcd_status = BSP_LCD_Init();
  if(lcd_status != LCD_OK){
	  printf("ERROR: L%d BSP_LCD_Init() failed.\r\n",__LINE__);
	  Error_Handler();
  }

  BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);

  /* Clear the LCD */
  BSP_LCD_Clear(LCD_COLOR_WHITE);

  /* Set LCD Example description */
  BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
  BSP_LCD_SetFont(&Font12);
  BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 20, (uint8_t *)"Copyright (c) Henryk and STMicroelectronics 2023", CENTER_MODE);
  BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
  BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), 120);
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
  BSP_LCD_SetFont(&Font24);

  snprintf(title,sizeof(title),"%s v%d.%02d", APP_NAME, APP_VERSION/100, APP_VERSION%100);

  BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)title, CENTER_MODE);
  BSP_LCD_SetFont(&Font16);
  BSP_LCD_DisplayStringAt(0, 60, (uint8_t *)"This example shows how to configure LCD", CENTER_MODE);
  BSP_LCD_DisplayStringAt(0, 75, (uint8_t *)"using both CubeMX and BSP library.", CENTER_MODE);

  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
  BSP_LCD_SetFont(&Font24);
}


/* USER CODE END 4 */

/* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0xC0000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_32MB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress = 0x90000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_64MB;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER3;
  MPU_InitStruct.BaseAddress = 0xA0000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_8KB;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  // from: STM32Cube_FW_F7_V1.17.1\Projects\STM32F769I-Discovery\Examples\BSP\Src\main.c
  /* Turn RED LED ON */
  HAL_GPIO_WritePin(LD_USER1_RED_GPIO_Port, LD_USER1_RED_Pin, GPIO_PIN_SET);
  /* Turn GREEN LEDs OFF */
  HAL_GPIO_WritePin(LD_USER2_GREEN_GPIO_Port, LD_USER2_GREEN_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LD3_GREEN_GPIO_Port, LD3_GREEN_Pin, GPIO_PIN_RESET);
  if(gUartStarted){
	  printf("L%d: %s() FATAL ERROR: Program halted.\r\n",__LINE__,__func__);
  }
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
