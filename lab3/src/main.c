/**

******************************************************************************
  * @file    GPIO/GPIO_EXTI/Src/main.c
  * @author  MCD Application Team
  * @version V1.8.0
  * @date    21-April-2017
  * @brief   This example describes how to configure and use GPIOs through
  *          the STM32L4xx HAL API.
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


this program: 

1. This project needs the libraray file i2c_at2464c.c and its header file. 
2. in the i2c_at2464c.c, the I2C SCL and SDA pins are configured as PULLUP. so do not need to pull up resistors (even do not need the 100 ohm resisters).
NOTE: students can also configure the TimeStamp pin 	

*/




/* Includes ------------------------------------------------------------------*/

#include "main.h"

/** @addtogroup STM32L4xx_HAL_Examples
  * @{
  */

/** @addtogroup GPIO_EXTI
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef  pI2c_Handle;

RTC_HandleTypeDef RTCHandle;     //creating instances to use 
RTC_DateTypeDef RTC_DateStructure;
RTC_TimeTypeDef RTC_TimeStructure;

__IO HAL_StatusTypeDef Hal_status;  //HAL_ERROR, HAL_TIMEOUT, HAL_OK, of HAL_BUSY 

//memory location to write to in the device
__IO uint16_t memLocation = 3; //pick any location within range
	uint16_t EE_status;

typedef enum MainState {TIMER, PREVTIME, RECTIME, DATE, SETTINGS} MainState; 
MainState STATE; 
typedef enum SelState {HELD,RELEASED} SelState; 
SelState SEL_STATE;
typedef enum SetVar {sss, smm, shh, syy, smo, sdd} SetVar;
SetVar SETVAR; 


char lcd_buffer[6];    // LCD display buffer
char timestring[10]={0};  //   
char datestring[6]={0};


// uint16_t EE_status;
uint8_t wd, dd, mo, yy, ss, mm, hh; // for weekday, day, month, year, second, minute, hour

__IO uint32_t SEL_Pressed_StartTick;   //sysTick when the User button is pressed

__IO uint8_t  rightpressed, uppressed, downpressed, selpressed;  // button pressed 
__IO uint8_t leftpressed=0;
__IO uint8_t  sel_held;   // if the selection button is held for a while (>800ms)

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);

void RTC_Config(void);
void RTC_AlarmAConfig(void);

void eeSaveTime();
void eeReadTime();
void eeClear();

void RTC_DateShow();
void RTC_TimeShow();

static void PushButton_Config(void);


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

	leftpressed=0;
	rightpressed=0;
	uppressed=0;
	downpressed=0;
	selpressed=0;
	sel_held=0;
	

	STATE=TIMER; 
	SETVAR=sss; 
	HAL_Init();
	
	BSP_LED_Init(LED4);
	BSP_LED_Init(LED5);
  
	SystemClock_Config();   
	PushButton_Config();
											
	
	
	HAL_InitTick(0x0000); //set the systick interrupt priority to the highest, !!!This line need to be after systemClock_config()

	
	BSP_LCD_GLASS_Init();
	
	BSP_JOY_Init(JOY_MODE_EXTI);

BSP_LCD_GLASS_DisplayString((uint8_t*)"LAB 3 :(");	
	HAL_Delay(400);



//configure real-time clock
	RTC_Config();

	
	RTC_AlarmAConfig();

	I2C_Init(&pI2c_Handle);


/*********************Testing I2C EEPROM------------------

	//the following variables are for testging I2C_EEPROM
	uint8_t data1 =0x67,  data2=0x68;
	uint8_t readData=0x00;
	uint16_t EE_status;




	EE_status=I2C_ByteWrite(&pI2c_Handle,EEPROM_ADDRESS, memLocation, data1);

  
  if(EE_status != HAL_OK)
  {
    I2C_Error(&pI2c_Handle);
  }
	
	
	BSP_LCD_GLASS_Clear();
	if (EE_status==HAL_OK) {
			BSP_LCD_GLASS_DisplayString((uint8_t*)"w 1 ok");
	}else
			BSP_LCD_GLASS_DisplayString((uint8_t*)"w 1 X");

	HAL_Delay(1000);
	
	EE_status=I2C_ByteWrite(&pI2c_Handle,EEPROM_ADDRESS, memLocation+1 , data2);
	
  if(EE_status != HAL_OK)
  {
    I2C_Error(&pI2c_Handle);
  }
	
	BSP_LCD_GLASS_Clear();
	if (EE_status==HAL_OK) {
			BSP_LCD_GLASS_DisplayString((uint8_t*)"w 2 ok");
	}else
			BSP_LCD_GLASS_DisplayString((uint8_t*)"w 2 X");

	HAL_Delay(1000);
	
	readData=I2C_ByteRead(&pI2c_Handle,EEPROM_ADDRESS, memLocation);

	BSP_LCD_GLASS_Clear();
	if (data1 == readData) {
			BSP_LCD_GLASS_DisplayString((uint8_t*)"r 1 ok");;
	}else{
			BSP_LCD_GLASS_DisplayString((uint8_t*)"r 1 X");
	}	
	
	HAL_Delay(1000);
	
	readData=I2C_ByteRead(&pI2c_Handle,EEPROM_ADDRESS, memLocation+1);

	BSP_LCD_GLASS_Clear();
	if (data2 == readData) {
			BSP_LCD_GLASS_DisplayString((uint8_t*)"r 2 ok");;
	}else{
			BSP_LCD_GLASS_DisplayString((uint8_t *)"r 2 X");
	}	

	HAL_Delay(1000);
	


******************************testing I2C EEPROM*****************************/


  /* Infinite loop */
  while (1)
  {
			//the joystick is pulled down. so the default status of the joystick is 0, when pressed, get status of 1. 
			//while the interrupt is configured at the falling edge---the moment the pressing is released, the interrupt is triggered.
			//therefore, the variable "selpressed==1" can not be used to make choice here.
			if (BSP_JOY_GetState() == JOY_SEL){
				SEL_Pressed_StartTick=HAL_GetTick();
					while(BSP_JOY_GetState() == JOY_SEL) {						//while the selection button is pressed)	
						if ((HAL_GetTick()-SEL_Pressed_StartTick)>1000){	
							RTC_AlarmA_IT_Disable(&RTCHandle);
							SEL_STATE=HELD;
							RTC_DateShow();
						}				
					}
			}
			

			switch (STATE) { 
				
				case TIMER:
					RTC_AlarmA_IT_Enable(&RTCHandle);
					break; 
				
				case PREVTIME:
						BSP_LCD_GLASS_Clear();
						BSP_LCD_GLASS_DisplayString((uint8_t*)"TIMES");
						HAL_Delay(400);
						eeReadTime();

					break;
				
				case RECTIME:
					eeSaveTime();
					STATE=TIMER;
					break; 
				
				case DATE:
					STATE=TIMER;
					break; 
				
				case SETTINGS:
					RTC_AlarmA_IT_Disable(&RTCHandle);
				
					switch(SETVAR){
						case sss:
							BSP_LCD_GLASS_Clear();
							BSP_LCD_GLASS_DisplayString((uint8_t*)"SEC");
							break;
						
						case smm:
							BSP_LCD_GLASS_Clear();
							BSP_LCD_GLASS_DisplayString((uint8_t*)"MIN");
							break;					
						
						case shh:
							BSP_LCD_GLASS_Clear();
							BSP_LCD_GLASS_DisplayString((uint8_t*)"HOUR");
							break;
						
						case syy:
							BSP_LCD_GLASS_Clear();
							BSP_LCD_GLASS_DisplayString((uint8_t*)"YEAR");
							break;
						
						case smo:
							BSP_LCD_GLASS_Clear();
							BSP_LCD_GLASS_DisplayString((uint8_t*)"MONTH");
							break;	
						
						case sdd:
							BSP_LCD_GLASS_Clear();
							BSP_LCD_GLASS_DisplayString((uint8_t*)"DAY");
							break;						
						
						
					}
					break; 
				
				default: 
					STATE=TIMER; 
				break; 
				
				
			} //end of switch					
		


	}
}


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follows :
  *            System Clock source            = MSI
  *            SYSCLK(Hz)                     = 4000000
  *            HCLK(Hz)                       = 4000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            Flash Latency(WS)              = 0
  * @param  None
  * @retval None
  */



void SystemClock_Config(void)
{ 
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};                                            

  // RTC requires to use HSE (or LSE or LSI, suspect these two are not available)
	//reading from RTC requires the APB clock is 7 times faster than HSE clock, 
	//so turn PLL on and use PLL as clock source to sysclk (so to APB)
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_MSI;     //RTC need either HSE, LSE or LSI           
  
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	
	RCC_OscInitStruct.MSIState = RCC_MSI_ON;  
	RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6; // RCC_MSIRANGE_6 is for 4Mhz. _7 is for 8 Mhz, _9 is for 16..., _10 is for 24 Mhz, _11 for 48Hhz
  RCC_OscInitStruct.MSICalibrationValue= RCC_MSICALIBRATION_DEFAULT;
  
	//RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;//RCC_PLL_NONE;

	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;   //PLL source: either MSI, or HSI or HSE, but can not make HSE work.
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40; 
  RCC_OscInitStruct.PLL.PLLR = 2;  //2,4,6 or 8
  RCC_OscInitStruct.PLL.PLLP = 7;   // or 17.
  RCC_OscInitStruct.PLL.PLLQ = 4;   //2, 4,6, 0r 8  
	//the PLL will be MSI (4Mhz)*N /M/R = 

	if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    // Initialization Error 
    while(1);
  }

  // configure the HCLK, PCLK1 and PCLK2 clocks dividers 
  // Set 0 Wait State flash latency for 4Mhz 
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; //the freq of pllclk is MSI (4Mhz)*N /M/R = 80Mhz 
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  
	
	if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)   //???
  {
    // Initialization Error 
    while(1);
  }

  // The voltage scaling allows optimizing the power consumption when the device is
  //   clocked below the maximum system frequency, to update the voltage scaling value
  //   regarding system frequency refer to product datasheet.  

  // Enable Power Control clock 
  __HAL_RCC_PWR_CLK_ENABLE();

  if(HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2) != HAL_OK)
  {
    // Initialization Error 
    while(1);
  }

  // Disable Power Control clock   //why disable it?
  __HAL_RCC_PWR_CLK_DISABLE();      
}
//after RCC configuration, for timmer 2---7, which are one APB1, the TIMxCLK from RCC is 4MHz


void RTC_Config(void) {
	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef RTC_DateStructure;
	

	//****1:***** Enable the RTC domain access (enable wirte access to the RTC )
			//1.1: Enable the Power Controller (PWR) APB1 interface clock:
        __HAL_RCC_PWR_CLK_ENABLE();    
			//1.2:  Enable access to RTC domain 
				HAL_PWR_EnableBkUpAccess();    
			//1.3: Select the RTC clock source
				__HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE);    
				//RCC_RTCCLKSOURCE_LSI is defined in hal_rcc.h
	       // according to P9 of AN3371 Application Note, LSI's accuracy is not suitable for RTC application!!!! 
				
			//1.4: Enable RTC Clock
			__HAL_RCC_RTC_ENABLE();   //enable RTC --see note for the Macro in _hal_rcc.h---using this Marco requires 
																//the above three lines.
			
	
			//1.5  Enable LSI
			__HAL_RCC_LSI_ENABLE();   //need to enable the LSI !!!
																//defined in _rcc.c
			while (__HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY)==RESET) {}    //defind in rcc.c
	
			// for the above steps, please see the CubeHal UM1725, p616, section "Backup Domain Access" 	
				
				
				
	//****2.*****  Configure the RTC Prescaler (Asynchronous and Synchronous) and RTC hour 
        
				RTCHandle.Instance = RTC; 
				RTCHandle.Init.HourFormat = RTC_HOURFORMAT_24;
				
				RTCHandle.Init.AsynchPrediv = 127; 
				RTCHandle.Init.SynchPrediv = 255; 
				
				
				RTCHandle.Init.OutPut = RTC_OUTPUT_ALARMA;
				RTCHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
				RTCHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
				
			
				if(HAL_RTC_Init(&RTCHandle) != HAL_OK)
				{
					BSP_LCD_GLASS_Clear(); 
					BSP_LCD_GLASS_DisplayString((uint8_t *)"RT I X"); 	
				}
	
	
	
	
	//****3.***** init the time and date
				
		
				RTC_DateStructure.Year = yy;	
				RTC_DateStructure.Month = mo;
				RTC_DateStructure.Date = dd;
				RTC_DateStructure.WeekDay = wd;
				
				if(HAL_RTC_SetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN) != HAL_OK)   //BIN format is better 
															//before, must set in BCD format and read in BIN format!!
				{
					BSP_LCD_GLASS_Clear();
					BSP_LCD_GLASS_DisplayString((uint8_t *)"D I X");
				} 
  
  
				RTC_TimeStructure.Hours = hh;  
				RTC_TimeStructure.Minutes = mm; 
				RTC_TimeStructure.Seconds = ss;
				RTC_TimeStructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
				RTC_TimeStructure.StoreOperation = RTC_STOREOPERATION_RESET;
				
				if(HAL_RTC_SetTime(&RTCHandle,&RTC_TimeStructure,RTC_FORMAT_BIN) != HAL_OK)   //BIN format is better
																																					//before, must set in BCD format and read in BIN format!!
				{
					BSP_LCD_GLASS_Clear();
					BSP_LCD_GLASS_DisplayString((uint8_t *)"T I X");
				}	
	  



				
			__HAL_RTC_TAMPER1_DISABLE(&RTCHandle);
			__HAL_RTC_TAMPER2_DISABLE(&RTCHandle);	
				//Optionally, a tamper event can cause a timestamp to be recorded. ---P802 of RM0090
				//Timestamp on tamper event
				//With TAMPTS set to �1 , any tamper event causes a timestamp to occur. In this case, either
				//the TSF bit or the TSOVF bit are set in RTC_ISR, in the same manner as if a normal
				//timestamp event occurs. The affected tamper flag register (TAMP1F, TAMP2F) is set at the
				//same time that TSF or TSOVF is set. ---P802, about Tamper detection
				//-------that is why need to disable this two tamper interrupts. Before disable these two, when program start, there is always a timestamp interrupt.
				//----also, these two disable function can not be put in the TSConfig().---put there will make  the program freezed when start. the possible reason is
				//-----one the RTC is configured, changing the control register again need to lock and unlock RTC and disable write protection.---See Alarm disable/Enable 
				//---function.
				
			HAL_RTC_WaitForSynchro(&RTCHandle);	
			//To read the calendar through the shadow registers after Calendar initialization,
			//		calendar update or after wake-up from low power modes the software must first clear
			//the RSF flag. The software must then wait until it is set again before reading the
			//calendar, which means that the calendar registers have been correctly copied into the
			//RTC_TR and RTC_DR shadow registers.The HAL_RTC_WaitForSynchro() function
			//implements the above software sequence (RSF clear and RSF check).	
}


void RTC_AlarmAConfig(void)
{
	RTC_AlarmTypeDef RTC_Alarm_Structure;
	
	RTC_Alarm_Structure.Alarm = RTC_ALARM_A;
  RTC_Alarm_Structure.AlarmMask = RTC_ALARMMASK_ALL;
	
	
				
  
  if(HAL_RTC_SetAlarm_IT(&RTCHandle,&RTC_Alarm_Structure,RTC_FORMAT_BCD) != HAL_OK)
  {
			BSP_LCD_GLASS_Clear(); 
			BSP_LCD_GLASS_DisplayString((uint8_t *)"A S X");
  }

	__HAL_RTC_ALARM_CLEAR_FLAG(&RTCHandle, RTC_FLAG_ALRAF); //without this line, sometimes(SOMETIMES, when first time to use the alarm interrupt)
																			//the interrupt handler will not work!!! 		

		//need to set/enable the NVIC for RTC_Alarm_IRQn!!!!
	HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);   
	HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 3, 0);  //not important ,but it is better not use the same prio as the systick
	
}

//You may need to disable and enable the RTC Alarm at some moment in your application
HAL_StatusTypeDef  RTC_AlarmA_IT_Disable(RTC_HandleTypeDef *hrtc) 
{ 
 	// Process Locked  
	__HAL_LOCK(hrtc);
  
  hrtc->State = HAL_RTC_STATE_BUSY;
  
  // Disable the write protection for RTC registers 
  __HAL_RTC_WRITEPROTECTION_DISABLE(hrtc);
  
  // __HAL_RTC_ALARMA_DISABLE(hrtc);
    
   // In case of interrupt mode is used, the interrupt source must disabled 
   __HAL_RTC_ALARM_DISABLE_IT(hrtc, RTC_IT_ALRA);


 // Enable the write protection for RTC registers 
  __HAL_RTC_WRITEPROTECTION_ENABLE(hrtc);
  
  hrtc->State = HAL_RTC_STATE_READY; 
  
  // Process Unlocked 
  __HAL_UNLOCK(hrtc);  
}


HAL_StatusTypeDef  RTC_AlarmA_IT_Enable(RTC_HandleTypeDef *hrtc) 
{	
	// Process Locked  
	__HAL_LOCK(hrtc);	
  hrtc->State = HAL_RTC_STATE_BUSY;
  
  // Disable the write protection for RTC registers 
  __HAL_RTC_WRITEPROTECTION_DISABLE(hrtc);
  
  // __HAL_RTC_ALARMA_ENABLE(hrtc);
    
   // In case of interrupt mode is used, the interrupt source must disabled 
   __HAL_RTC_ALARM_ENABLE_IT(hrtc, RTC_IT_ALRA);


 // Enable the write protection for RTC registers 
  __HAL_RTC_WRITEPROTECTION_ENABLE(hrtc);
  
  hrtc->State = HAL_RTC_STATE_READY; 
  
  // Process Unlocked 
  __HAL_UNLOCK(hrtc);  

}

static void PushButton_Config(void)
{
  GPIO_InitTypeDef   GPIO_InitStructure;

  /* Enable GPIOC clock */
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /* Configure PE.15 pin as input*/
  GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStructure.Pull = GPIO_PULLUP;
  GPIO_InitStructure.Pin = GPIO_PIN_15 | GPIO_PIN_14;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	
  /* Enable and set EXTI lines 10 to 15 Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 4, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}


/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  switch (GPIO_Pin) {
			case GPIO_PIN_0: 					//SELECT button	
						if(SEL_STATE == HELD){
							SEL_STATE=RELEASED;
							STATE=TIMER;
						} else STATE=RECTIME; 
						break;	
			case GPIO_PIN_1:     //left button
							if(STATE == SETTINGS){
								SETVAR=(SETVAR+1)%6;
							} else if (STATE == PREVTIME) {
								STATE=TIMER;
							} else {
							STATE=PREVTIME; 
							}
							break;
			case GPIO_PIN_2:    //right button						  to play again.
							/*if(STATE == SETTINGS) STATE=TIMER;
							else STATE=SETTINGS;
							//rightpressed+=1;		*/
							break;
			case GPIO_PIN_3:    //up button						
							if(STATE == SETTINGS){
								switch(SETVAR){
									case sss:
										ss+=1;
									  sprintf(lcd_buffer,"SEC=%02u",ss);
										BSP_LCD_GLASS_Clear();
										BSP_LCD_GLASS_DisplayString((uint8_t*)lcd_buffer);
										HAL_Delay(1000);
										break;
									
									case smm:
										mm+=1;
									  sprintf(lcd_buffer,"MIN=%02u",mm);
										BSP_LCD_GLASS_Clear();
										BSP_LCD_GLASS_DisplayString((uint8_t*)lcd_buffer);
										HAL_Delay(1000);
										break;					
									
									case shh:
										hh+=1;
									  sprintf(lcd_buffer,"HRS=%02u",hh);
										BSP_LCD_GLASS_Clear();
										BSP_LCD_GLASS_DisplayString((uint8_t*)lcd_buffer);
										HAL_Delay(1000);
										break;
									
									case syy:
										yy+=1;
									  sprintf(lcd_buffer,"YR=%02u",yy);
										BSP_LCD_GLASS_Clear();
										BSP_LCD_GLASS_DisplayString((uint8_t*)lcd_buffer);
										HAL_Delay(1000);
										break;
									
									case smo:
										mo+=1;
									  sprintf(lcd_buffer,"MON=%02u",mo);
										BSP_LCD_GLASS_Clear();
										BSP_LCD_GLASS_DisplayString((uint8_t*)lcd_buffer);
										HAL_Delay(1000);
										break;	
									
									case sdd:
										dd+=1;
									  sprintf(lcd_buffer,"DAY=%02u",dd);
										BSP_LCD_GLASS_Clear();
										BSP_LCD_GLASS_DisplayString((uint8_t*)lcd_buffer);
										HAL_Delay(1000);
										break;						

								}
							}
							RTC_Config();
							break;
			
			case GPIO_PIN_5:    //down button						
if(STATE == SETTINGS){
								switch(SETVAR){
									case sss:
										if(ss == 0) ss=0;
										else ss-=1;
									  sprintf(lcd_buffer,"SEC=%02u",ss);
										BSP_LCD_GLASS_Clear();
										BSP_LCD_GLASS_DisplayString((uint8_t*)lcd_buffer);
										HAL_Delay(1000);
										break;
									
									case smm:
										if(mm == 0) mm=0;
										else mm-=1;
									  sprintf(lcd_buffer,"MIN=%02u",mm);
										BSP_LCD_GLASS_Clear();
										BSP_LCD_GLASS_DisplayString((uint8_t*)lcd_buffer);
										HAL_Delay(1000);
										break;					
									
									case shh:
										if(hh == 0) hh=0;
										else hh-=1;
									  sprintf(lcd_buffer,"HRS=%02u",hh);
										BSP_LCD_GLASS_Clear();
										BSP_LCD_GLASS_DisplayString((uint8_t*)lcd_buffer);
										HAL_Delay(1000);
										break;
									
									case syy:
										if(yy == 0) yy=0;
										else yy-=1;
									  sprintf(lcd_buffer,"YR=%02u",yy);
										BSP_LCD_GLASS_Clear();
										BSP_LCD_GLASS_DisplayString((uint8_t*)lcd_buffer);
										HAL_Delay(1000);
										break;
									
									case smo:
										if(mo == 0) ss=0;
										else mo-=1;
									  sprintf(lcd_buffer,"MON=%02u",mo);
										BSP_LCD_GLASS_Clear();
										BSP_LCD_GLASS_DisplayString((uint8_t*)lcd_buffer);
										HAL_Delay(1000);
										break;	
									
									case sdd:
										if(dd == 0) dd=0;
										else dd-=1;
									  sprintf(lcd_buffer,"DAY=%02u",dd);
										BSP_LCD_GLASS_Clear();
										BSP_LCD_GLASS_DisplayString((uint8_t*)lcd_buffer);
										HAL_Delay(1000);
										break;						

								}
							}
							RTC_Config();
							break;
							
			case GPIO_PIN_14:    //down button						
							STATE=RECTIME;
							break;			
			case GPIO_PIN_15:
							if(STATE == SETTINGS) STATE=TIMER;
							else STATE=SETTINGS;
			default:
						//default
						break;
	  } 
}


void RTC_TimeShow(RTC_HandleTypeDef *hrtc)   //Funtion to display the RTC time on the LCD
	{
	HAL_RTC_GetTime(hrtc, &RTC_TimeStructure, RTC_FORMAT_BIN); //updates hh,mm,ss every sec
	HAL_RTC_GetDate(hrtc, &RTC_DateStructure, RTC_FORMAT_BIN); 
		
	// sets time vatiable to variables stored in the rtc time structure 
	ss = RTC_TimeStructure.Seconds; 
	mm = RTC_TimeStructure.Minutes; 
	hh = RTC_TimeStructure.Hours; 
		
	// converts values to a string to be displayed on the LCD 
	sprintf(timestring,"%02u%02u%02u",hh,mm,ss);
	BSP_LCD_GLASS_Clear();
	BSP_LCD_GLASS_DisplayString((uint8_t*)timestring);	
		
	

}
	
void eeSaveTime()
{
	if (memLocation >= 65532)
	{
		BSP_LCD_GLASS_DisplayString((uint8_t*)"FULL");
	} 
	else {
		BSP_LCD_GLASS_Clear();
		BSP_LCD_GLASS_DisplayString((uint8_t*)"SAVING");
		
		HAL_RTC_GetTime(&RTCHandle, &RTC_TimeStructure, RTC_FORMAT_BIN); //updates hh,mm,ss every sec
		HAL_RTC_GetDate(&RTCHandle, &RTC_DateStructure, RTC_FORMAT_BIN); 
			
		// sets time vatiable to variables stored in the rtc time structure 
		ss = RTC_TimeStructure.Seconds; 
		mm = RTC_TimeStructure.Minutes; 
		hh = RTC_TimeStructure.Hours; 
		
		
		
		EE_status=I2C_ByteWrite(&pI2c_Handle,EEPROM_ADDRESS, memLocation,hh);
		
		if(EE_status != HAL_OK)
		{
			I2C_Error(&pI2c_Handle);
			BSP_LCD_GLASS_Clear();
			BSP_LCD_GLASS_DisplayString((uint8_t*)"ERROR");

		}
		memLocation++;
		HAL_Delay(100);
		
		
		EE_status=I2C_ByteWrite(&pI2c_Handle,EEPROM_ADDRESS, memLocation,mm);
		
		if(EE_status != HAL_OK)
		{
			I2C_Error(&pI2c_Handle);
		}
		memLocation++;
		HAL_Delay(100);
		
	EE_status=I2C_ByteWrite(&pI2c_Handle,EEPROM_ADDRESS, memLocation,ss);
		
		if(EE_status != HAL_OK)
		{
			I2C_Error(&pI2c_Handle);
		}
			memLocation++;
			HAL_Delay(100);
		
		BSP_LCD_GLASS_Clear();
		BSP_LCD_GLASS_DisplayString((uint8_t*)"SAVED");
		HAL_Delay(400);
	}
}

void eeClear()
	{
	for( int i=0; i<20; i++){
		EE_status=I2C_ByteWrite(&pI2c_Handle,EEPROM_ADDRESS, memLocation,0x00);
		
		if(EE_status != HAL_OK)
		{
			I2C_Error(&pI2c_Handle);

		}
		memLocation++;
		HAL_Delay(100);
	}
	memLocation=0x0A;
}
void eeReadTime()
{
	if (memLocation <= 3)
		{
			BSP_LCD_GLASS_Clear();
			BSP_LCD_GLASS_DisplayString((uint8_t*)"EMPTY");
			HAL_Delay(400);
		} else {
																																						//try to for loop this?
			ss=I2C_ByteRead(&pI2c_Handle,EEPROM_ADDRESS, memLocation-1);
			HAL_Delay(10);
			mm=I2C_ByteRead(&pI2c_Handle,EEPROM_ADDRESS, memLocation-2);
			HAL_Delay(10);
			hh=I2C_ByteRead(&pI2c_Handle,EEPROM_ADDRESS, memLocation-3);
			sprintf(timestring,"%02u%02u%02u",hh,mm,ss);
			BSP_LCD_GLASS_Clear();
			BSP_LCD_GLASS_DisplayString((uint8_t*)timestring);	
			HAL_Delay(1000);
			
			ss=I2C_ByteRead(&pI2c_Handle,EEPROM_ADDRESS, memLocation-4);
			HAL_Delay(10);
			mm=I2C_ByteRead(&pI2c_Handle,EEPROM_ADDRESS, memLocation-5);
			HAL_Delay(10);
			hh=I2C_ByteRead(&pI2c_Handle,EEPROM_ADDRESS, memLocation-6);
			sprintf(timestring,"%02u%02u%02u",hh,mm,ss);
			BSP_LCD_GLASS_Clear();
			BSP_LCD_GLASS_DisplayString((uint8_t*)timestring);	
			HAL_Delay(1000);
			
		}
		

}		


void RTC_DateShow()  //Funtion to display the RTC date on the LCD
	{
	HAL_RTC_GetDate(&RTCHandle, &RTC_DateStructure, RTC_FORMAT_BIN); 
 
	yy=RTC_DateStructure.Year;
	mo=RTC_DateStructure.Month;
	dd=RTC_DateStructure.Date;

		
	sprintf(datestring,"%02u%02u%02u",yy,mo,dd);
	BSP_LCD_GLASS_Clear();
	BSP_LCD_GLASS_DisplayString((uint8_t*)datestring);	
	HAL_Delay(400);
		
}
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
	if(STATE == TIMER){
		BSP_LED_Toggle(LED5);
		RTC_TimeShow(hrtc);
	}
	
}

static void Error_Handler(void)
{
  /* Turn LED4 on */
  BSP_LED_On(LED4);
  while(1)
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
