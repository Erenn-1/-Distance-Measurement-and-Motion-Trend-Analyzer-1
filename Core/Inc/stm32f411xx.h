#ifndef STM32F411XX_H
#define STM32F411XX_H

/*
 * Saf bare-metal STM32F411xE register tanımları.
 * Hiçbir harici kütüphane (HAL, LL, CMSIS core_cm4.h) kullanılmaz.
 * Tüm adresler ve yapılar RM0383 referans kılavuzundan alınmıştır.
 */

#include <stdint.h>   /* uint8_t, uint32_t — sadece standart C türleri */

/* ===================================================================
 * CORTEX-M4 ÇEKİRDEK ÇEVRE BİRİMLERİ (Cortex-M4 Core Peripherals)
 * Taban adres: 0xE000E000  (System Control Space - SCS)
 * =================================================================== */

#define SCS_BASE     0xE000E000UL   /* System Control Space başlangıcı */

/* ----- SysTick (0xE000E010) ----- */
typedef struct {
    volatile uint32_t CTRL;    /* 0x00: Kontrol ve Durum Kaydı (CLKSOURCE, TICKINT, ENABLE, COUNTFLAG) */
    volatile uint32_t LOAD;    /* 0x04: Yeniden Yükleme Değeri (geri sayım başlangıcı) */
    volatile uint32_t VAL;     /* 0x08: Anlık Sayaç Değeri (yazma → sıfırlar) */
    volatile uint32_t CALIB;   /* 0x0C: Kalibrasyon Kaydı (salt okunur) */
} SysTick_Type;

#define SysTick_BASE  (SCS_BASE + 0x0010UL)
#define SysTick       ((SysTick_Type *)SysTick_BASE)

/* ----- NVIC — Nested Vectored Interrupt Controller (0xE000E100) ----- */
typedef struct {
    volatile uint32_t ISER[8U];    /* 0x000: Interrupt Set-Enable Registers   */
    uint32_t RESERVED0[24U];
    volatile uint32_t ICER[8U];    /* 0x080: Interrupt Clear-Enable Registers  */
    uint32_t RESERVED1[24U];
    volatile uint32_t ISPR[8U];    /* 0x100: Interrupt Set-Pending Registers   */
    uint32_t RESERVED2[24U];
    volatile uint32_t ICPR[8U];    /* 0x180: Interrupt Clear-Pending Registers */
    uint32_t RESERVED3[24U];
    volatile uint32_t IABR[8U];    /* 0x200: Interrupt Active Bit Registers    */
    uint32_t RESERVED4[56U];
    volatile uint8_t  IP[240U];    /* 0x300: Interrupt Priority Registers (8-bit each) */
    uint32_t RESERVED5[644U];
    volatile uint32_t STIR;        /* 0xE00: Software Trigger Interrupt Register */
} NVIC_Type;

#define NVIC_BASE  (SCS_BASE + 0x0100UL)
#define NVIC       ((NVIC_Type *)NVIC_BASE)

/* ----- SCB — System Control Block (0xE000ED00) ----- */
typedef struct {
    volatile uint32_t CPUID;      /* 0x000: CPUID Taban Kaydı (salt okunur)              */
    volatile uint32_t ICSR;       /* 0x004: Interrupt Control and State Register          */
    volatile uint32_t VTOR;       /* 0x008: Vektör Tablo Offset Kaydı                    */
    volatile uint32_t AIRCR;      /* 0x00C: Application Interrupt and Reset Control       */
    volatile uint32_t SCR;        /* 0x010: System Control Register                       */
    volatile uint32_t CCR;        /* 0x014: Configuration and Control Register            */
    volatile uint8_t  SHP[12U];   /* 0x018: System Handler Priority Registers (8-bit)    */
    volatile uint32_t SHCSR;      /* 0x024: System Handler Control and State Register     */
    volatile uint32_t CFSR;       /* 0x028: Configurable Fault Status Register            */
    volatile uint32_t HFSR;       /* 0x02C: HardFault Status Register                    */
    volatile uint32_t DFSR;       /* 0x030: Debug Fault Status Register                  */
    volatile uint32_t MMFAR;      /* 0x034: MemManage Fault Address Register              */
    volatile uint32_t BFAR;       /* 0x038: BusFault Address Register                    */
    volatile uint32_t AFSR;       /* 0x03C: Auxiliary Fault Status Register               */
} SCB_Type;

#define SCB_BASE  (SCS_BASE + 0x0D00UL)
#define SCB       ((SCB_Type *)SCB_BASE)

/* SysTick CTRL bit maskeleri (system.c tarafından kullanılır) */
#define SYSTICK_CTRL_ENABLE    (1U << 0)   /* Sayacı başlat */
#define SYSTICK_CTRL_TICKINT   (1U << 1)   /* Sıfırda SysTick_Handler'ı çağır */
#define SYSTICK_CTRL_CLKSOURCE (1U << 2)   /* 1=işlemci saati (HCLK), 0=HCLK/8 */

/* ===================================================================
 * RCC — Reset and Clock Control  (0x40023800)
 * APB1 ve AHB1 saat etkinleştirme bitleri
 * =================================================================== */
typedef struct {
    volatile uint32_t CR;           /* 0x00 */
    volatile uint32_t PLLCFGR;      /* 0x04 */
    volatile uint32_t CFGR;         /* 0x08 */
    volatile uint32_t CIR;          /* 0x0C */
    volatile uint32_t AHB1RSTR;     /* 0x10 */
    volatile uint32_t AHB2RSTR;     /* 0x14 */
    uint32_t RESERVED0[2];          /* 0x18-0x1C */
    volatile uint32_t APB1RSTR;     /* 0x20 */
    volatile uint32_t APB2RSTR;     /* 0x24 */
    uint32_t RESERVED1[2];          /* 0x28-0x2C */
    volatile uint32_t AHB1ENR;      /* 0x30 */
    volatile uint32_t AHB2ENR;      /* 0x34 */
    uint32_t RESERVED2[2];          /* 0x38-0x3C */
    volatile uint32_t APB1ENR;      /* 0x40 */
    volatile uint32_t APB2ENR;      /* 0x44 */
    uint32_t RESERVED3[2];          /* 0x48-0x4C */
    volatile uint32_t AHB1LPENR;    /* 0x50 */
    volatile uint32_t AHB2LPENR;    /* 0x54 */
    uint32_t RESERVED4[2];          /* 0x58-0x5C */
    volatile uint32_t APB1LPENR;    /* 0x60 */
    volatile uint32_t APB2LPENR;    /* 0x64 */
    uint32_t RESERVED5[2];          /* 0x68-0x6C */
    volatile uint32_t BDCR;         /* 0x70 */
    volatile uint32_t CSR;          /* 0x74 */
} RCC_TypeDef;

#define RCC_BASE             0x40023800UL
#define RCC                  ((RCC_TypeDef *)RCC_BASE)

#define RCC_AHB1ENR_GPIOAEN  (1U << 0)   /* GPIOA saat izni (AHB1ENR bit 0) */
#define RCC_APB1ENR_TIM2EN   (1U << 0)   /* TIM2 saat izni (APB1ENR bit 0)  */
#define RCC_APB1ENR_USART2EN (1U << 17)  /* USART2 saat izni (APB1ENR bit 17) */

/* ===================================================================
 * GPIO — General Purpose Input/Output  (GPIOA: 0x40020000)
 * =================================================================== */
typedef struct {
    volatile uint32_t MODER;    /* 0x00: Pin modu (00=Giriş, 01=Çıkış, 10=AF, 11=Analog) */
    volatile uint32_t OTYPER;   /* 0x04: Çıkış tipi (0=Push-pull, 1=Open-drain) */
    volatile uint32_t OSPEEDR;  /* 0x08: Çıkış hızı */
    volatile uint32_t PUPDR;    /* 0x0C: Pull-up/Pull-down */
    volatile uint32_t IDR;      /* 0x10: Giriş verisi (salt okunur) */
    volatile uint32_t ODR;      /* 0x14: Çıkış verisi */
    volatile uint32_t BSRR;     /* 0x18: Bit set/reset (üst 16 bit=reset, alt 16 bit=set) */
    volatile uint32_t LCKR;     /* 0x1C: Kilit kaydı */
    volatile uint32_t AFR[2];   /* 0x20-0x24: Alternate Function (AFR[0]=pin0-7, AFR[1]=pin8-15) */
} GPIO_TypeDef;

#define GPIOA_BASE  0x40020000UL
#define GPIOA       ((GPIO_TypeDef *)GPIOA_BASE)

/* ===================================================================
 * TIM2 — 32-bit General Purpose Timer  (0x40000000)
 * =================================================================== */
typedef struct {
    volatile uint32_t CR1;     /* 0x00: Kontrol kaydı 1 (CEN, DIR, vb.) */
    volatile uint32_t CR2;     /* 0x04: Kontrol kaydı 2 */
    volatile uint32_t SMCR;    /* 0x08: Slave mode kontrol */
    volatile uint32_t DIER;    /* 0x0C: DMA/Interrupt enable */
    volatile uint32_t SR;      /* 0x10: Durum kaydı (UIF bayrağı burada) */
    volatile uint32_t EGR;     /* 0x14: Event generation (UG biti: PSC shadow yükleme) */
    volatile uint32_t CCMR1;   /* 0x18: Capture/Compare mode 1 */
    volatile uint32_t CCMR2;   /* 0x1C: Capture/Compare mode 2 */
    volatile uint32_t CCER;    /* 0x20: Capture/Compare enable */
    volatile uint32_t CNT;     /* 0x24: Sayaç değeri (32-bit, 1 µs/tick @ 1 MHz) */
    volatile uint32_t PSC;     /* 0x28: Prescaler (shadow register — UG ile yüklenir) */
    volatile uint32_t ARR;     /* 0x2C: Auto-reload değeri */
    uint32_t RESERVED;         /* 0x30: Ayrılmış (gelişmiş timer'larda RCR) */
    volatile uint32_t CCR1;    /* 0x34: Capture/Compare kaydı 1 */
    volatile uint32_t CCR2;    /* 0x38: Capture/Compare kaydı 2 */
    volatile uint32_t CCR3;    /* 0x3C: Capture/Compare kaydı 3 */
    volatile uint32_t CCR4;    /* 0x40: Capture/Compare kaydı 4 */
} TIM_TypeDef;

#define TIM2_BASE    0x40000000UL
#define TIM2         ((TIM_TypeDef *)TIM2_BASE)

#define TIM_CR1_CEN  (1U << 0)   /* Counter Enable */
#define TIM_EGR_UG   (1U << 0)   /* Update Generation: PSC shadow register'ı hemen yükle */

/* ===================================================================
 * USART2  (0x40004400)
 * =================================================================== */
typedef struct {
    volatile uint32_t SR;    /* 0x00: Durum kaydı (TXE, RXNE, vb.) */
    volatile uint32_t DR;    /* 0x04: Veri kaydı (okuma=RX, yazma=TX) */
    volatile uint32_t BRR;   /* 0x08: Baud Rate kaydı */
    volatile uint32_t CR1;   /* 0x0C: Kontrol 1 (UE, TE, RE) */
    volatile uint32_t CR2;   /* 0x10: Kontrol 2 */
    volatile uint32_t CR3;   /* 0x14: Kontrol 3 */
    volatile uint32_t GTPR;  /* 0x18: Guard time ve prescaler */
} USART_TypeDef;

#define USART2_BASE      0x40004400UL
#define USART2           ((USART_TypeDef *)USART2_BASE)

#define USART_SR_RXNE    (1U << 5)    /* Read Data Register Not Empty */
#define USART_SR_TXE     (1U << 7)    /* Transmit Data Register Empty */
#define USART_CR1_RE     (1U << 2)    /* Receiver Enable */
#define USART_CR1_TE     (1U << 3)    /* Transmitter Enable */
#define USART_CR1_UE     (1U << 13)   /* USART Enable */

#endif /* STM32F411XX_H */
