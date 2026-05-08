#ifndef STM32F411XX_H
#define STM32F411XX_H

/* Cortex-M4 core configuration for STM32F411xE (required by core_cm4.h) */
#define __CM4_REV              0x0001U
#define __MPU_PRESENT          1U
#define __NVIC_PRIO_BITS       4U
#define __Vendor_SysTickConfig 0U
#define __FPU_PRESENT          1U
#include "core_cm4.h"   /* Provides SysTick_Type, SysTick pointer, and uint32_t */

/* ===== RCC ===== */
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t PLLCFGR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t AHB1RSTR;
    volatile uint32_t AHB2RSTR;
    uint32_t RESERVED0[2];
    volatile uint32_t APB1RSTR;
    volatile uint32_t APB2RSTR;
    uint32_t RESERVED1[2];
    volatile uint32_t AHB1ENR;
    volatile uint32_t AHB2ENR;
    uint32_t RESERVED2[2];
    volatile uint32_t APB1ENR;
    volatile uint32_t APB2ENR;
    uint32_t RESERVED3[2];
    volatile uint32_t AHB1LPENR;
    volatile uint32_t AHB2LPENR;
    uint32_t RESERVED4[2];
    volatile uint32_t APB1LPENR;
    volatile uint32_t APB2LPENR;
    uint32_t RESERVED5[2];
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
} RCC_TypeDef;

#define RCC_BASE            0x40023800UL
#define RCC                 ((RCC_TypeDef *)RCC_BASE)

#define RCC_AHB1ENR_GPIOAEN (1U << 0)
#define RCC_APB1ENR_TIM2EN  (1U << 0)
#define RCC_APB1ENR_USART2EN (1U << 17)

/* ===== GPIO ===== */
typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
} GPIO_TypeDef;

#define GPIOA_BASE          0x40020000UL
#define GPIOA               ((GPIO_TypeDef *)GPIOA_BASE)

/* ===== TIM2 (32-bit general purpose timer) ===== */
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
    uint32_t RESERVED;
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
} TIM_TypeDef;

#define TIM2_BASE           0x40000000UL
#define TIM2                ((TIM_TypeDef *)TIM2_BASE)

#define TIM_CR1_CEN         (1U << 0)
#define TIM_EGR_UG          (1U << 0)   /* Update Generation: forces PSC/ARR shadow register reload */

/* ===== USART2 ===== */
typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} USART_TypeDef;

#define USART2_BASE         0x40004400UL
#define USART2              ((USART_TypeDef *)USART2_BASE)

#define USART_SR_RXNE       (1U << 5)
#define USART_SR_TXE        (1U << 7)
#define USART_CR1_RE        (1U << 2)
#define USART_CR1_TE        (1U << 3)
#define USART_CR1_UE        (1U << 13)

/* ===== SysTick control bits ===== */
/* SysTick_Type and SysTick pointer come from core_cm4.h above */
#define SYSTICK_CTRL_ENABLE    (1U << 0)
#define SYSTICK_CTRL_TICKINT   (1U << 1)
#define SYSTICK_CTRL_CLKSOURCE (1U << 2)

#endif /* STM32F411XX_H */
