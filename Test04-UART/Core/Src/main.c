/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : HAMAN
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define RX_BUF_SIZE  40
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch)  // printf
{
    HAL_UART_Transmit(&huart2, &ch, 1, 10);
    return ch;
}
int __io_getchar(void)  // console input (scanf)
{
    char ch = 0;

    //while(HAL_UART_Receive(&huart2, &ch, 1, 10) != HAL_OK);
    while(1)
    {
        HAL_StatusTypeDef r = HAL_UART_Receive(&huart2, &ch, 1, 10);
        if(r == HAL_OK) break;
    }
    HAL_UART_Transmit(&huart2, &ch, 1, 10);  // echo
    return ch;
}
int scanfEx1(int *k)  // INT VALUE
{
    unsigned char buf[10], c;
    int idx = 0;

    while(1)
    {
        c = __io_getchar();
        if(c == 0x0d) break;
        if(c < '0' || c > '9') break;
        if(idx > 9) break;
        buf[idx++] = c;
    }
    buf[idx] = 0;
    *k = atoi(buf);
    return 1;
}
int GetKey()  // interface function
{
    return __io_getchar();
}
char rxBuf1[RX_BUF_SIZE], rx;  //UART1
char rxBuf2[RX_BUF_SIZE];      //UART2
char dmaBuf[RX_BUF_SIZE];
int  head = 0, tail = 0;
int  rxIdx1 = 0 ,rxIdx2=0;
int op = -1; // 0 ~ 3 : NULL ,CR ,CRLF ,LFCR
char *EL[] = { "\0" ,"\r" ,"\r\n" ,"\n\r" };

char * GetDmaData() // uart2
{
    int len = 0;
    tail = RX_BUF_SIZE - huart2.hdmarx->Instance->NDTR;

    if(head == tail) return 0;
    if(tail > head)
    {
        memcpy(dmaBuf, rxBuf2 + head, tail - head); len = tail - head;
    }
    else // tail < head
    {
        memcpy(dmaBuf, rxBuf2 + head, RX_BUF_SIZE - head);
        memcpy(dmaBuf + RX_BUF_SIZE - head, rxBuf2, tail);
        len = RX_BUF_SIZE - head + tail;
    }
    dmaBuf[len] = 0;
    head = tail;
    return dmaBuf;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == &huart1)
    {
        rxBuf1[rxIdx1++] = rx;
        if(rx == '\0' || rx == '\r' || rx == '\n')
        {
          HAL_UART_Transmit(&huart2, rxBuf1, rxIdx1, 10);
          HAL_UART_Transmit(&huart2, "\r\n", 2, 10);            //putty display out
          rxIdx1 = 0;
        }
    }
    else if(huart == &huart2)
    {
        rxBuf2[rxIdx2++] = rx;
        if(rx == '\0' || rx == '\r' || rx == '\n') //BT > AT ??
        {
          if(strncmp(rxBuf2 ,"BT>" ,3) == 0)
          {
            HAL_UART_Transmit(&huart1, rxBuf2 + 3, rxIdx2 - 4 ,10);
            if(op>0) HAL_UART_Transmit(&huart1, EL[op], strlen(EL[op]), 10);
            HAL_UART_Transmit(&huart2, rxBuf2 + 3, rxIdx2 - 4 ,10);
            HAL_UART_Transmit(&huart2, "\r\n",2, 10);
          }
          else
          {
            HAL_UART_Transmit(&huart2, rxBuf2 + 3, rxIdx2 - 4 ,10);
            HAL_UART_Transmit(&huart2, "\r\n", 2, 10);
          }
          rxIdx2 = 0;
        }
    }
     HAL_UART_Receive_IT(huart, &rx, 1);
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
      //char *str = GetDmaData();  // str = NULL, if no data
      if(rxIdx1)
      {
          printf("%s\r\n", rxBuf1); rxIdx1 = 0;
      }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == B1_Pin)
    {
      char *str = GetDmaData();  // str = NULL, if no data
      if(str)
      {
          printf("[Receive Buffer] %s\r\n", rxBuf2);
          printf("[Receive String] %s\r\n\n", str);
      }
    }
}

void LED_CONTROL()
{
      int k = GetKey();
      switch(k)
      {
      case 'r':
      case 'R':
          HAL_GPIO_WritePin(LDr_GPIO_Port, LDr_Pin, 1); HAL_Delay(200);
          HAL_GPIO_WritePin(LDr_GPIO_Port, LDr_Pin, 0); HAL_Delay(200);
          break;
      case 'g':
      case 'G':
          HAL_GPIO_WritePin(LDg_GPIO_Port, LDg_Pin, 1); HAL_Delay(200);
          HAL_GPIO_WritePin(LDg_GPIO_Port, LDg_Pin, 0); HAL_Delay(200);
          break;
      case 'y':
      case 'Y':
          HAL_GPIO_WritePin(LDy_GPIO_Port, LDy_Pin, 1); HAL_Delay(200);
          HAL_GPIO_WritePin(LDy_GPIO_Port, LDy_Pin, 0); HAL_Delay(200);
          break;
      default:
          break;
      }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  printf("\033[1J\033[1;1HUART Program Started....\r\n");
  while(1)
  {
      printf("Input End-Line option\r\n");
      printf("  '0' : NULL\r\n");
      printf("  '1' : CR\r\n");
      printf("  '2' : CRLF\r\n");
      printf("  '3' : LFCR\r\n");

      scanfEx1(&op);
      if(op >= 0 && op < 4) break;
  }
  printf("OK....%d\r\n",op);

  HAL_TIM_Base_Start_IT(&htim3);
  //HAL_UART_Receive_DMA(&huart2, rxBuf, RX_BUF_SIZE);
  HAL_UART_Receive_IT(&huart2, &rx, 1);
  HAL_UART_Receive_IT(&huart1, &rx, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // LED_CONTROL();  // LED Toggle by KBD
    //  if(rxIdx)
    //  {
    //    rxBuf[rxIdx] = 0;
    //    printf("[Received String] %s\r\n", rxBuf);
    //    rxIdx = 0;
    //  }
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 8400-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 10000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|LDy_Pin|LDg_Pin|LDr_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin LDy_Pin LDg_Pin LDr_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|LDy_Pin|LDg_Pin|LDr_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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
