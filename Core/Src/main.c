#include "stm32f411xe.h" // Cihaza ait register tanımlarını (CMSIS) içerir

/* ==============================================================================
 * PROJE: Mesafe Ölçüm ve UART Haberleşme Sistemi
 * TAKIM: Mustafa (Kişi A) & Eren (Kişi B)
 * DONANIM: STM32F411RE Nucleo-64
 * ============================================================================== */


// ==============================================================
// [GÜN 2] KİŞİ A (MUSTAFA) - SysTick Timer ile Donanımsal Bekleme
// ==============================================================
void delay_us(uint32_t us) {
    SysTick->VAL = 0;                     // Mevcut sayacı sıfırla
    SysTick->LOAD = (us * 16) - 1;        // 16 MHz clock için 1 us = 16 vuruş
    SysTick->CTRL = 5;                    // SysTick'i başlat (Processor Clock, Enable)
    while (!(SysTick->CTRL & (1 << 16))); // Süre dolana kadar bekle (COUNTFLAG)
    SysTick->CTRL = 0;                    // İş bitince SysTick'i durdur
}

// ==============================================================
// [GÜN 3] KİŞİ A (MUSTAFA) - TIM2 Input Capture (Sensör Yankısı İçin)
// ==============================================================
void TIM2_Input_Capture_Init(void) {
    RCC->APB1ENR |= (1 << 0);       // 1. TIM2 Clock aktif et
    TIM2->PSC = 15;                 // 2. Prescaler: 16MHz / (15+1) = 1 MHz (1 us hassasiyet)
    TIM2->ARR = 0xFFFFFFFF;         // 3. ARR: 32-bit timer maksimum değer
    TIM2->CCMR1 |= (1 << 0);        // 4. Kanal 1'i (CH1) Input olarak ayarla
    TIM2->CCMR1 &= ~(1 << 1);
    TIM2->CCER |= (1 << 0);         // 5. Yakalama işlemini (Capture) aktif et
    TIM2->CR1 |= (1 << 0);          // 6. Kronometreyi başlat (CEN biti)
}

// ==============================================================
// [GÜN 4] KİŞİ A (MUSTAFA) - USART2 Bilgisayar Haberleşmesi (9600 bps)
// ==============================================================
void USART2_Init(void) {
    RCC->AHB1ENR |= (1 << 0);       // GPIOA Clock aktif et
    RCC->APB1ENR |= (1 << 17);      // USART2 Clock aktif et

    // PA2(TX) ve PA3(RX) Alternatif Fonksiyon (AF) moduna al
    GPIOA->MODER &= ~(0xF << 4);    // 4,5,6,7. bitleri temizle
    GPIOA->MODER |= (0xA << 4);     // 1010 (AF modu)

    // AF7 (USART2) seçimi yap
    GPIOA->AFR[0] |= (0x77 << 8);

    // Baud Rate: 9600 bps (@ 16 MHz)
    USART2->BRR = 0x0683;

    // USART2, TX ve RX aktif et
    USART2->CR1 |= (1 << 13) | (1 << 3) | (1 << 2);
}

// Tek karakter gönderme fonksiyonu
void UART2_SendChar(char c) {
    while (!(USART2->SR & (1 << 7))); // TX Buffer boşalana kadar bekle
    USART2->DR = c;                   // Karakteri yolla
}

// String (Metin) gönderme fonksiyonu
void UART2_SendString(char* string) {
    while (*string) {
        UART2_SendChar(*string++);
    }
}


// ==============================================================
// ANA PROGRAM (MAIN)
// ==============================================================
int main(void) {

    // ----------------------------------------------------------
    // KURULUM (SETUP) - Yalnızca 1 kez çalışır
    // ----------------------------------------------------------

    TIM2_Input_Capture_Init(); // [GÜN 3] Mustafa: Timer Kurulumu
    USART2_Init();             // [GÜN 4] Mustafa: UART Kurulumu

    // [GÜN 1] Eren: Donanım Ayarları (PA5 LED Çıkışı)
    GPIOA->MODER &= ~(3 << 10);
    GPIOA->MODER |= (1 << 10);


    // ----------------------------------------------------------
    // ANA DÖNGÜ (LOOP) - Sonsuza dek çalışır
    // ----------------------------------------------------------
    while (1) {

        // [GÜN 1] Eren: LED'i yakıp söndür
        GPIOA->ODR ^= (1 << 5);

        // [GÜN 4] Mustafa: Bilgisayara yaşam belirtisi gönder
        UART2_SendString("STM32 Calisiyor! \r\n");

        // [GÜN 2] Mustafa: Profesyonel donanımsal bekleme
        delay_us(1000000);

    }
}
