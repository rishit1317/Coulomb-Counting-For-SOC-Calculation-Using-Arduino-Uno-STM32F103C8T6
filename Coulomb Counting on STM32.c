/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body with 18A E-Fuse Protection
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Battery configuration */
#define BATTERY_CAPACITY_AH     9.0f
#define BATTERY_CAPACITY_AS     (BATTERY_CAPACITY_AH * 3600.0f)

/* ADC configuration */
#define VREF                    3.3f
#define ADC_RES                 4095.0f

/* Current-sense setup */
#define VDIV_SCALE              1.0f

/* NCV84045 current sense */
#define R_SENSE                 150.0f
#define CS_RATIO                1415.0f

/* Noise threshold */
#define NOISE_THRESHOLD_V       0.015f

/* ========== 18A E-FUSE PROTECTION THRESHOLDS ========== */
#define NORMAL_LOAD_MAX_A       5.0f      /* Normal operation: 0-5A */
#define CAUTION_THRESHOLD_A     8.0f      /* Caution: 8A (44% of fuse) */
#define WARNING_THRESHOLD_A     12.0f     /* Warning: 12A (67% of fuse) */
#define CRITICAL_THRESHOLD_A    15.0f     /* Critical: 15A (83% of fuse) */
#define EFUSE_RATING_A          18.0f     /* E-Fuse rating */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN PV */

/* ========== TIMING VARIABLES ========== */
volatile uint32_t last_tick = 0;
volatile uint32_t dt_ms = 0;

/* ========== CURRENT & BATTERY MONITORING ========== */
volatile float    current_a           = 0.0f;
volatile float    soc_percent         = 100.0f;
volatile float    remaining_charge_as = BATTERY_CAPACITY_AS;

/* ========== ADC VARIABLES ========== */
volatile float    adc_voltage         = 0.0f;
volatile float    actual_cs_voltage   = 0.0f;
volatile uint32_t raw_adc             = 0;

/* ========== SHORT CIRCUIT PROTECTION STATUS ========== */
typedef enum {
  STATUS_NORMAL      = 0,    /* 0-5A: Normal operation */
  STATUS_CAUTION     = 1,    /* 5-8A: Caution zone */
  STATUS_WARNING     = 2,    /* 8-12A: Warning zone */
  STATUS_CRITICAL    = 3,    /* 12-15A: Critical zone */
  STATUS_EXTREME     = 4,    /* 15-18A: Extreme (near fuse trip) */
  STATUS_FUSE_BLOWN  = 5     /* >18A: Fuse blown */
} system_status_t;

volatile system_status_t system_status = STATUS_NORMAL;
volatile uint8_t  emergency_shutdown  = 0;
volatile float    fuse_percentage     = 0.0f;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief  Read ADC value from current sense pin
  * @param  hadc: ADC handle pointer
  * @retval Raw ADC value (0-4095)
  */
static uint32_t ADC_Read(ADC_HandleTypeDef *hadc)
{
  HAL_ADC_Start(hadc);
  HAL_ADC_PollForConversion(hadc, 10);
  uint32_t val = HAL_ADC_GetValue(hadc);
  HAL_ADC_Stop(hadc);
  return val;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  float dt_s;
  float used_charge;
  uint32_t now;
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
  MX_ADC1_Init();

  /* USER CODE BEGIN 2 */

  /* Calibrate ADC (MUST be done BEFORE first conversion) */
  HAL_ADCEx_Calibration_Start(&hadc1);

  /* Initialize timing for coulomb counting */
  last_tick = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* ========== GET TIME DELTA ========== */
    now = HAL_GetTick();
    dt_ms = now - last_tick;
    last_tick = now;

    /* ========== READ ADC ========== */
    raw_adc = ADC_Read(&hadc1);

    /* Clamp ADC value to valid range */
    if (raw_adc > 4095U)
    {
      raw_adc = 4095U;
    }

    /* Convert raw ADC to voltage: V = (raw / 4095) * 3.3V */
    adc_voltage = ((float)raw_adc / ADC_RES) * VREF;

    /* Apply voltage divider correction (if any) */
    actual_cs_voltage = adc_voltage * VDIV_SCALE;

    /* ========== NOISE HANDLING ========== */
    if (actual_cs_voltage < NOISE_THRESHOLD_V)
    {
      actual_cs_voltage = 0.0f;
      current_a = 0.0f;
    }
    else
    {
      /* Calculate current from sense voltage */
      /* I = (V_out / R_sense) * Gain */
      current_a = (actual_cs_voltage / R_SENSE) * CS_RATIO;
    }

    /* ========== CALCULATE FUSE PERCENTAGE ========== */
    fuse_percentage = (current_a / EFUSE_RATING_A) * 100.0f;

    /* ========== 18A E-FUSE PROTECTION LOGIC ========== */
    if (current_a > EFUSE_RATING_A)
    {
      /* FUSE BLOWN: Current exceeded 18A rating */
      system_status = STATUS_FUSE_BLOWN;
      emergency_shutdown = 1;
    }
    else if (current_a > CRITICAL_THRESHOLD_A)
    {
      /* EXTREME: 15A-18A (83%-100% of fuse) */
      system_status = STATUS_EXTREME;
      emergency_shutdown = 1;
    }
    else if (current_a > WARNING_THRESHOLD_A)
    {
      /* CRITICAL: 12A-15A (67%-83% of fuse) */
      system_status = STATUS_CRITICAL;
      emergency_shutdown = 0;
    }
    else if (current_a > CAUTION_THRESHOLD_A)
    {
      /* WARNING: 8A-12A (44%-67% of fuse) */
      system_status = STATUS_WARNING;
      emergency_shutdown = 0;
    }
    else if (current_a > NORMAL_LOAD_MAX_A)
    {
      /* CAUTION: 5A-8A (28%-44% of fuse) */
      system_status = STATUS_CAUTION;
      emergency_shutdown = 0;
    }
    else
    {
      /* NORMAL: 0-5A (0%-28% of fuse) */
      system_status = STATUS_NORMAL;
      emergency_shutdown = 0;
    }

    /* ========== COULOMB COUNTING ========== */
    dt_s = (float)dt_ms / 1000.0f;  /* Convert milliseconds to seconds */
    used_charge = current_a * dt_s;  /* Charge = Current * Time (Ampere-seconds) */

    /* Subtract used charge from remaining */
    if (remaining_charge_as > used_charge)
    {
      remaining_charge_as -= used_charge;
    }
    else
    {
      remaining_charge_as = 0.0f;
    }

    /* Calculate State of Charge (SOC) as percentage */
    soc_percent = (remaining_charge_as / BATTERY_CAPACITY_AS) * 100.0f;

    /* Clamp SOC to valid range [0%, 100%] */
    if (soc_percent > 100.0f)
    {
      soc_percent = 100.0f;
    }

    if (soc_percent < 0.0f)
    {
      soc_percent = 0.0f;
    }

    /* Wait 1 second before next measurement */
    HAL_Delay(1000);

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel (PA0 = ADC_CHANNEL_0)
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;

  /* Use longer sampling time for stable ADC reading */
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* Configure PA0 as analog input for current sense */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
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
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
