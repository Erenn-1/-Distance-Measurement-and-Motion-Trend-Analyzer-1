#include "stm32f411xe.h" // Cihaza ait register tanımlarını (CMSIS) içerir

// ==============================================================
// KİŞİ A (MUSTAFA) - Gün 2 Görevi: SysTick Timer ile Bekleme
// ==============================================================
void delay_us(uint32_t us) {
    SysTick->VAL = 0;
    SysTick->LOAD = (us * 16) - 1;
    SysTick->CTRL = 5;
    while (!(SysTick->CTRL & (1 << 16)));
    SysTick->CTRL = 0;
}

// ==============================================================
// KİŞİ A (MUSTAFA) - Gün 3 Görevi: TIM2 Input Capture Ayarları
// ==============================================================
void TIM2_Input_Capture_Init(void) {

    // 1. TIM2'nin şalterini (clock sinyalini) açıyoruz (APB1 hattındadır)
    RCC->APB1ENR |= (1 << 0);

    // 2. Prescaler (PSC) Ayarı: 16 MHz / (15 + 1) = 1 MHz (1 mikrosaniye hassasiyet)
    TIM2->PSC = 15;

    // 3. Auto-Reload Register (ARR): 32-bit timer
    TIM2->ARR = 0xFFFFFFFF;

    // 4. CCMR1 Register: Kanal 1'i (CH1) "Giriş (Input)" olarak seçiyoruz.
    TIM2->CCMR1 |= (1 << 0);
    TIM2->CCMR1 &= ~(1 << 1);

    // 5. CCER Register: Yakalama işlemini aktif ediyoruz (Kamerayı açıyoruz)
    TIM2->CCER |= (1 << 0);

    // 6. CR1 Register: Kronometreyi resmen başlatıyoruz! (CEN biti)
    TIM2->CR1 |= (1 << 0);
}


// ==============================================================
// ANA PROGRAM (İşlemcinin ilk girdiği yer)
// ==============================================================
int main(void) {

    // ==============================================================
    // 1. KURULUM KISMI (Sadece 1 kez çalışır)
    // ==============================================================

    // A portunun saat sinyalini aktif et
    RCC->AHB1ENR |= (1 << 0);

    // KRONOMETREYİ ÇALIŞTIR (İşte bunu buraya yazman gerekiyordu!)
    TIM2_Input_Capture_Init();


    // --- EREN'İN DONANIM AYARLARI ---
    // PA5 pinini Output (Çıkış) moduna al
    GPIOA->MODER &= ~(3 << 10);
    GPIOA->MODER |= (1 << 10);


    // ==============================================================
    // 2. ANA DÖNGÜ (Sonsuza dek çalışır)
    // ==============================================================
    while (1) {

        // PA5'in durumunu tersine çevir (LED'i yak/söndür)
        GPIOA->ODR ^= (1 << 5);

        // Profesyonel Bekleme
        delay_us(1000000);

    }
}
