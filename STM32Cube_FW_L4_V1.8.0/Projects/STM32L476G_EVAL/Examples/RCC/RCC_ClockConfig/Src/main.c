/**
  ******************************************************************************
  * @file    RCC/RCC_ClockConfig/Src/main.c
  * @author  MCD Application Team
  * @version V1.8.0
  * @date    21-April-2017
  * @brief   This example describes how to use the RCC HAL API to configure the
  *          system clock (SYSCLK) and modify the clock settings on run time.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32L4xx_HAL_Examples
  * @{
  */

/** @addtogroup RCC_ClockConfig
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO FlagStatus SwitchClock = RESET;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void SystemClockHSI_Config(void);
static void SystemClockHSE_Config(void);
static void SystemClockMSI_Config(void);
static void SwitchSystemClock(void);
static void Error_Handler(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /* STM32L4xx HAL library initialization:
       - Configure the Flash prefetch
       - Systick timer is configured by default as source of time base, but user 
         can eventually implement his proper time base source (a general purpose 
         timer for example or other time source), keeping in mind that Time base 
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
  HAL_Init();

  /* Enable HSE oscillator and configure the PLL to reach the max system frequency (80 MHz)
     when using HSE oscillator as PLL clock source. */
  SystemClock_Config();
  
  /* Configure LED1 and LED3 */
  BSP_LED_Init(LED1);
  BSP_LED_Init(LED3);

  /* Initialize Wkup/Tamper push-button, will be used to trigger an interrupt each time it's pressed.*/
  /* In the ISR the PLL source will be changed from MSI to HSI circularly */
  BSP_PB_Init(BUTTON_TAMPER, BUTTON_MODE_EXTI);

  /* Output SYSCLK  on MCO1 pin(PA.08) */
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_SYSCLK, RCC_MCODIV_1);

  /* Toggle some leds in an infinite loop */
  while (1)
  {
    /* check if user button has been pressed to switch clock config */
    if(SwitchClock != RESET)
    {
      SwitchSystemClock();
    }
    
    /* Toggle LED1 */
    BSP_LED_Toggle(LED1);
    HAL_Delay(100);
  }
}


/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == TAMPER_BUTTON_PIN)
  {
    SwitchClock = SET;
  }
}


/**
  * @brief  switch in system clock out of ISR context.
  * @retval None
  */
static void SwitchSystemClock(void)
{
  if (__HAL_RCC_GET_PLL_OSCSOURCE() == RCC_PLLSOURCE_HSI)
  {
    /* PLL source is HSI oscillator */
    /* Set SYSCLK frequency to 80000000 Hz, coming from the PLL which is clocked by MSI */
    SystemClockMSI_Config();
  }
  else if (__HAL_RCC_GET_PLL_OSCSOURCE() == RCC_PLLSOURCE_HSE)
  {
    /* PLL source is HSE  oscillator */
    /* Set SYSCLK frequency to 80000000 Hz, coming from the PLL which is clocked by HSI */
    SystemClockHSI_Config();
  }
  else if (__HAL_RCC_GET_PLL_OSCSOURCE() == RCC_PLLSOURCE_MSI)
  {
    /* PLL source is MSI oscillator */
    /* Set SYSCLK frequency to 80000000 Hz, coming from the PLL which is clocked by HSE  */
    SystemClockHSE_Config();
  }

  /* reset global variable */
  SwitchClock = RESET;
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follows :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 1
  *            PLL_N                          = 20
  *            PLL_P                          = 7
  *            PLL_Q                          = 4
  *            PLL_R                          = 2
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  /* Enable HSE Oscillator and activate PLL with HSE as source   */
  /* (Default MSI Oscillator enabled at system reset remains ON) */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLP = 7;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
}

/**
  * @brief  Switch the PLL source from MSI to HSE , and select the PLL as SYSCLK
  *         source.
  *         The system Clock is configured as follows :
  *            System Clock source            = PLL (HSE )
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 1
  *            PLL_N                          = 20
  *            PLL_P                          = 7
  *            PLL_Q                          = 4
  *            PLL_R                          = 2
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
static void SystemClockHSE_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  /* -1- Select MSI as system clock source to allow modification of the PLL configuration */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  
  /* -2- Enable HSE  Oscillator, select it as PLL source and finally activate the PLL */
  RCC_OscInitStruct.OscillatorType        = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState              = RCC_HSE_ON;
  
  RCC_OscInitStruct.PLL.PLLSource         = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLState          = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLM              = 1;
  RCC_OscInitStruct.PLL.PLLN              = 20;
  RCC_OscInitStruct.PLL.PLLP              = 7;
  RCC_OscInitStruct.PLL.PLLQ              = 4;
  RCC_OscInitStruct.PLL.PLLR              = 2;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* -3- Select the PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType       = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource    = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider   = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider  = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider  = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* -4- Optional: Disable MSI Oscillator (if the MSI is no more needed by the application)*/
  RCC_OscInitStruct.OscillatorType  = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState        = RCC_MSI_OFF;
  RCC_OscInitStruct.PLL.PLLState    = RCC_PLL_NONE;  /* No update on PLL */
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
}

/**
  * @brief  Switch the PLL source from HSE  to HSI, and select the PLL as SYSCLK
  *         source.
  *         The system Clock is configured as follows :
  *            System Clock source            = PLL (HSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            HSI Frequency(Hz)              = 16000000
  *            PLLM                           = 2
  *            PLLN                           = 20
  *            PLLP                           = 7
  *            PLLQ                           = 4
  *            PLLR                           = 2
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
static void SystemClockHSI_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  /* -1- Select HSE  as system clock source to allow modification of the PLL configuration */
  RCC_ClkInitStruct.ClockType       = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource    = RCC_SYSCLKSOURCE_HSE;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* -2- Enable HSI Oscillator, select it as PLL source and finally activate the PLL */
  RCC_OscInitStruct.OscillatorType    = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState          = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue  = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState      = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource     = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM          = 2;
  RCC_OscInitStruct.PLL.PLLN          = 20;
  RCC_OscInitStruct.PLL.PLLP          = 7;
  RCC_OscInitStruct.PLL.PLLQ          = 4;
  RCC_OscInitStruct.PLL.PLLR          = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* -3- Select the PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType       = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource    = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider   = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider  = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider  = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* -4- Optional: Disable MSI Oscillator */
  RCC_OscInitStruct.OscillatorType  = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState        = RCC_MSI_OFF;
  RCC_OscInitStruct.PLL.PLLState    = RCC_PLL_NONE;  /* No update on PLL */
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
}

/**
  * @brief  Switch the PLL source from HSI to MSI, and select the PLL as SYSCLK
  *         source.
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 40
  *            PLL_P                          = 7
  *            PLL_Q                          = 4
  *            PLL_R                          = 2
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
static void SystemClockMSI_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  /* -1- Select HSI as system clock source to allow modification of the PLL configuration */
  RCC_ClkInitStruct.ClockType       = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource    = RCC_SYSCLKSOURCE_HSI;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

  /* -2- Enable MSI Oscillator, select it as PLL source and finally activate the PLL */
  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState            = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange       = RCC_MSIRANGE_6;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM            = 1;
  RCC_OscInitStruct.PLL.PLLN            = 40;
  RCC_OscInitStruct.PLL.PLLP            = 7;
  RCC_OscInitStruct.PLL.PLLQ            = 4;
  RCC_OscInitStruct.PLL.PLLR            = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* -3- Select the PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType       = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource    = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider   = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider  = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider  = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

  /* -4- Optional: Disable HSI Oscillator (if the HSI is no more needed by the application)*/
  RCC_OscInitStruct.OscillatorType  = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState        = RCC_HSI_OFF;
  RCC_OscInitStruct.PLL.PLLState    = RCC_PLL_NONE;  /* No update on PLL */
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* Turn LED4 on */
  BSP_LED_On(LED3);

  while (1)
  {
  }
}
 
#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
