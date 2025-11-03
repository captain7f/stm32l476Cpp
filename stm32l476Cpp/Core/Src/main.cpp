/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include <GPIO_Hal.h>
#include "UART_Hal.h"
#include "i2c_Hal.h"
#include "main.h"
#include "gpio.h"
#include "tim_Hal.h"
#include <stdio.h>
#include <string.h>



void SystemClock_Config(void);

/* USER CODE BEGIN 0 */
volatile uint32_t blueB_latch= 0;
void blueB_callback(){
	blueB_latch++;
}


extern "C" int main(void)
{


  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

//  Button buttonBlue(blueButton_GPIO_Port, blueButton_Pin);

  // ld2 using interrupt from Button blue
  Led ld2(LED_ON_BOARD_GPIO_Port, LED_ON_BOARD_Pin);
  ButtonISR ButtonBlue(GPIOC, GPIO_PIN_13);
  ButtonBlue.set_isr_cb(blueB_callback);
  uint32_t blueB_state= 0;

  // Uart
  UartBase uart3(USART3, &huart3);
  UartIT uartIt2(USART2, &huart2);
//  UartDMA uartIt2(USART1, &huart1); // DMA Uart1

  uint8_t c[64]= "Pragram start\n\r";
  uartIt2.write(c, strlen((char*)c));
  while(!uartIt2.is_tx_done());
  memset(c,0,sizeof(c));
  uartIt2.start_read();

  // timer3
  // 1. Create the Timer_Base object (assuming 'htim3' is the HAL handle)
  TimBase timer3(TIM3, &htim3);
  // 2. Start the timer
  timer3.start();
  // timer3
  TimIt timer2(TIM2, &htim2);
  uint64_t tick_ms_pre = 0;
  timer2.start();

  // i2c
  // i2c lm75
  clLm75 i2clm(&hi2c1, I2C1);
  float temperature_celsius = 0.0f;
  if (HAL_I2C_IsDeviceReady(&hi2c1, (0x48<<1), 3, 100) == HAL_OK) {
	  uint8_t msg[] = "✅ LM75 (0x48) bereit\r\n";
	  uartIt2.write(msg, strlen((char*)msg));
  } else {
	  uint8_t msg[] = "❌ LM75 (0x48) NICHT bereit\r\n";
      uartIt2.write(msg, strlen((char*)msg));
  }
  // i2c EEprom
  clEeprom i2cEEP(&hi2c2, I2C2);
  // Prüfen, ob EEPROM antwortet
	if (HAL_I2C_IsDeviceReady(&hi2c2, EEPROM_ADDR_8B, 3, 100) == HAL_OK) {
	   printf("✅ EEPROM (0x50) antwortet!\r\n");
	} else {
	   printf("❌ EEPROM antwortet NICHT!\r\n");
	}

   // (Optional) Teststring schreiben
   i2cEEP.EEPROM_WriteText(0x0100, "Hello from STM32 via I2C2!");

   // Gelesene Daten an PuTTY ausgeben:
   printf("Dump ab 0x0100 (64 Bytes):\r\n");
   i2cEEP.EEPROM_PrintBlock(0x0100, 64);

  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  // -----------toggle the ld2 Led--------------------
//	  ld2.toggle();
//	  HAL_Delay(5000);

	  // toggle the ld2 using blue Button
//	  ld2.write(buttonBlue.readButtonState());

	  // toggle the ld2 using Interrupt from blue Button
	  if(blueB_state != blueB_latch){
		  HAL_Delay(500);

		  blueB_state = blueB_latch;
		  ld2.toggle();
	  }


	  // ---------------Uart2--------------------
	  uint16_t len= uartIt2.read(c, sizeof(c));

	  if(len>0){
		  while(!uartIt2.is_tx_done());
		  uartIt2.write(c, len);
	  }
	  memset(c,0,sizeof(c));


    // --------------Timer3------------------------

	  uint32_t start_time = timer3.read();
	  while (timer3.read() - start_time < 1500);
	  // timer2 interrupt
	  if(timer2.delay_Ms(tick_ms_pre, 1000)){
		  ld2.toggle();
	  }
	  tick_ms_pre = timer2.read();

	  // i2c1 lm75
	  uint8_t msg[]= "Error\n\r";
	  HAL_StatusTypeDef status = i2clm.ReadLM75Temperature(LM75_ADDR, &temperature_celsius);
	  if(status == HAL_OK) printf("Temperatur (LM75): %d C\n", (uint8_t)temperature_celsius);
//	  else printf("Temperatur (LM75) Error ");
	  else uartIt2.write(msg, strlen((char*)msg));
  }

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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
