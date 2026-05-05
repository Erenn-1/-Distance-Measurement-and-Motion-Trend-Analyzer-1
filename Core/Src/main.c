#include "stm32f411xe.h" // Cihaza ait register tanımlarını (CMSIS) içerir
#include <stdio.h>       // Gün 5: Sayıları metne çeviren sprintf() için eklendi

// ==============================================================
// GÜN 2, 3 VE 4'TE YAZDIĞIN ALTYAPI FONKSİYONLARI
// (Bunlar olmadan loglama yapamayız)
// ==============================================================
void delay_us(uint32_t us) {
    SysTick->VAL = 0;
    SysTick->LOAD = (us * 16) - 1;
    SysTick->CTRL = 5;
    while (!(SysTick->CTRL & (1 << 16)));
    SysTick->CTRL = 0;
}

void TIM2_Input_Capture_Init(void) {
    RCC->APB1ENR |= (1 << 0);
    TIM2->PSC = 15;
    TIM2->ARR = 0xFFFFFFFF;
    TIM2->CCMR1 |= (1 << 0);
    TIM2->CCMR1 &= ~(1 << 1);
    TIM2->CCER |= (1 << 0);
    TIM2->CR1 |= (1 << 0);
}

void USART2_Init(void) {
    RCC->AHB1ENR |= (1 << 0);
    RCC->APB1ENR |= (1 << 17);
    GPIOA->MODER &= ~(0xF << 4);
    GPIOA->MODER |= (0xA << 4);
    GPIOA->AFR[0] |= (0x77 << 8);
    USART2->BRR = 0x0683;
    USART2->CR1 |= (1 << 13) | (1 << 3) | (1 << 2);
}

void UART2_SendChar(char c) {
    while (!(USART2->SR & (1 << 7)));
    USART2->DR = c;
}

void UART2_SendString(char* string) {
    while (*string) { UART2_SendChar(*string++); }
}


// ==============================================================
// EREN'İN GÜN 5 İÇİN YAZACAĞI SENSÖR FONKSİYONLARI (Taslak)
// ==============================================================
void HCSR04_Init(void) {
    // Eren buraya Trig ve Echo pinlerinin donanım ayarlarını yazacak
}

uint32_t HCSR04_Measure(void) {
    // Eren buraya Timer ile mesafeyi hesaplayan matematiği yazacak
    // Şimdilik sistemin çalıştığını görmek için sahte bir değer döndürelim:
    return 34; // Sensör 34 cm ölçmüş gibi farz edelim
}


// ==============================================================
// ANA PROGRAM (Gün 5 Akışı)
// ==============================================================
int main(void) {

    // ----------------------------------------------------------
    // 1. INIT (Kurulum) AŞAMASI
    // ----------------------------------------------------------

    // Senin orijinal kodundaki pin ayarları
    // 1. GPIOA için Clock'u aktif et (Enable GPIOA clock)
    RCC->AHB1ENR |= (1 << 0);

    // 2. PA5 pinini Output moduna al (Configure PA5 as output)
    GPIOA->MODER &= ~(3 << 10);
    GPIOA->MODER |= (1 << 10);

    // Diğer modüllerin kurulumları
    TIM2_Input_Capture_Init(); // Mustafa: Kronometre
    USART2_Init();             // Mustafa: PC Haberleşmesi
    HCSR04_Init();             // Eren: Sensör pinleri

    // Gün 5 Loglama değişkenleri
    char log_buffer[50];       // Metni tutacak kutu
    uint32_t distance_cm = 0;  // Mesafeyi tutacak değişken


    // ----------------------------------------------------------
    // 2. LOOP (Döngü) AŞAMASI
    // ----------------------------------------------------------
    while (1) {

        // 3. PA5'in durumunu tersine çevir (Toggle LED)
        GPIOA->ODR ^= (1 << 5);

        // ------------------------------------------------------
        // 3. MEASURE (Ölçüm) AŞAMASI
        // ------------------------------------------------------
        distance_cm = HCSR04_Measure(); // Eren'in kodundan değeri çekiyoruz

        // ------------------------------------------------------
        // 4. LOG (Kayıtlama) AŞAMASI
        // ------------------------------------------------------
        // Sayıyı metne çevir ve bilgisayara fırlat
        sprintf(log_buffer, "Mesafe: %lu cm \r\n", distance_cm);
        UART2_SendString(log_buffer);

        // 4. Gecikme
        // Eski "dummy delay" for döngüsü yerine senin Gün 2'de yazdığın
        // donanımsal delay_us fonksiyonunu kullanıyoruz ki işlemci tam süresinde uyusun.
        delay_us(500000); // 500 ms (Yarım saniye) bekle
    }
}
