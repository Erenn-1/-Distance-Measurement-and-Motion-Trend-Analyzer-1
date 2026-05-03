#include "stm32f411xe.h" // Cihaza ait register tanımlarını (CMSIS) içerir

// ==============================================================
// KİŞİ A (MUSTAFA) - Gün 2 Görevi: SysTick Timer ile Bekleme
// ==============================================================
void delay_us(uint32_t us) {
    SysTick->VAL = 0;                     // 1. Mevcut sayacı sıfırla
    SysTick->LOAD = (us * 16) - 1;        // 2. 16 MHz clock için 1 us = 16 vuruş
    SysTick->CTRL = 5;                    // 3. SysTick'i başlat (Clock source: Processor, Enable)
    while (!(SysTick->CTRL & (1 << 16))); // 4. Süre dolana kadar bekle (COUNTFLAG kontrolü)
    SysTick->CTRL = 0;                    // 5. İş bitince SysTick'i durdur
}

int main(void) {

    // ==============================================================
    // KİŞİ A (MUSTAFA) - Gün 2 Görevi: Clock Konfigürasyonları
    // ==============================================================

    // 1. GPIOA için Clock'u aktif et (NUCLEO üzerindeki LED PA5'e bağlıdır)
    // AHB1ENR register'ının 0. bitini (GPIOAEN) 1 yapıyoruz.
    RCC->AHB1ENR |= (1 << 0);


    // --- EREN'İN DONANIM AYARLARI ---

    // 2. PA5 pinini Output (Çıkış) moduna al
    // Önce 10. ve 11. bitleri temizliyoruz (00 yapıyoruz)
    GPIOA->MODER &= ~(3 << 10);
    // Sonra 10. biti 1 yapıyoruz (01 -> General purpose output mode)
    GPIOA->MODER |= (1 << 10);


    // ==============================================================
    // ANA DÖNGÜ
    // ==============================================================
    while (1) {

        // 3. PA5'in durumunu tersine çevir (LED'i yak/söndür)
        GPIOA->ODR ^= (1 << 5);

        // 4. Profesyonel Bekleme (Senin yazdığın donanımsal timer)
        delay_us(1000000); // 1.000.000 mikrosaniye = 1 tam saniye bekle

    }
}
