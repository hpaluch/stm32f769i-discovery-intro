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
#include <inttypes.h> // printf(3) format macros for stdtypes.h, for example PRIu32 to print uint32_t
#include <stdbool.h> // bool type and true/false
#include <machine/endian.h> // __bswap16(), __htons(), __ntohs()
// BSP includes
#include "stm32f769i_discovery.h"
#include "stm32f769i_discovery_sdram.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define APP_VERSION 105 // 123=1.23
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
/* USER CODE BEGIN PFP */
// Redirecting printf(3) to UART1 that is connected to ST-Link Virtual COM port
// from: STM32Cube_FW_F7_V1.17.0\Projects\STM32F767ZI-Nucleo\Examples\UART\UART_Printf\Src\main.c
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


// TODO: Once CPU Cache is enabled we have to invalidate cache at least After Write
// and before verification
bool app_test_sdram(int *addr, int bytes, int seed)
{
	uint32_t tick1,tick2,tick3;
	int i,v;
	int items = bytes/sizeof(int);

	tick1 = HAL_GetTick();
	printf("L%d Test SDRAM sizeof(int)=%d start_addr=%p bytes=%d items=%d seed=%d\r\n",
			__LINE__,(int)sizeof(int),addr,bytes,items,seed);
	srand(seed);
	printf("- L%d SDRAM test started...\r\n",__LINE__);
	for(i=0;i<items;i++){
		v = rand();
		if (i==0){
			printf("  - Writing at %p: %d",addr,v);
		} else if (i >=1 && i<7){
			printf(" %d",v);
		} else if (i==7){
			printf("...\r\n");
		} /* else { // noisy
			printf(" i=%d",i);
			fflush(stdout);
		}*/
		addr[i] = v;
	}
	tick2= HAL_GetTick();
	printf("- L%d Write finished in %" PRIu32 " [ms]\r\n", __LINE__, tick2-tick1);
	printf("- L%d SDRAM - reading back data...\r\n",__LINE__);
	srand(seed); // start again same series
	for(i=0;i<items;i++){
		v = rand();
		if (addr[i]!=v){
			printf("ERROR: Data at %p: %d <> %d\r\n",addr+i,addr[i],v);
			return false;
		}
	}
	tick3= HAL_GetTick();
	printf("- L%d read-back finished in %" PRIu32 " [ms]\r\n",__LINE__,tick3-tick2);
	printf("- OK: Read data matching Written data. seed=%d\r\n",seed);
	printf("L%d Write+Read of %d.%03d [kB] finished in %" PRIu32 " [ms]\r\n",
			__LINE__,bytes/1000,bytes%1000,tick3-tick1);
	return true;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  unsigned loopCount = 0;
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

  if (!app_test_sdram((int*)SDRAM_DEVICE_ADDR, SDRAM_DEVICE_SIZE ,1)){
	  printf("ERROR: L%d app_test_sdram() failed.\r\n",__LINE__);
	  Error_Handler();
  }

  // try different seed to ensure that we are not reading old (good) data...
  // make it a bit shorter...
  if (!app_test_sdram((int*)SDRAM_DEVICE_ADDR, 65536 /*APP_SDRAM_DEVICE_SIZE*/ ,10)){
	  printf("ERROR: L%d app_test_sdram() failed.\r\n",__LINE__);
	  Error_Handler();
  }

  printf("L%d Press Blue User button to continue to LEDs loop...\r\n",__LINE__);
  // wait until button is pressed
  while(HAL_GPIO_ReadPin(B_USER_GPIO_Port, B_USER_Pin)== GPIO_PIN_RESET); // NOP
  printf("L%d button pressed - entering LEDs loop.\r\n", __LINE__);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	// try to not overflow UART output - once per second is enough.
	if (loopCount%10 == 0){
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
