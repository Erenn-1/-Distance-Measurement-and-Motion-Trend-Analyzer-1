#include "stm32f4xx.h"  // Cihaza ait register tanımlarını (CMSIS) içerir

int main(void) {

    // 1. GPIOA için Clock'u aktif et (Enable GPIOA clock)
    // AHB1ENR register'ının 0. bitini (GPIOAEN) 1 yapıyoruz.
    RCC->AHB1ENR |= (1 << 0);

    // 2. PA5 pinini Output moduna al (Configure PA5 as output)
    // Önce 10. ve 11. bitleri temizliyoruz (00 yapıyoruz)
    GPIOA->MODER &= ~(3 << 10);
    // Sonra 10. biti 1 yapıyoruz (01 -> General purpose output mode)
    GPIOA->MODER |= (1 << 10);

    // Sonsuz Döngü
    while (1) {

        // 3. PA5'in durumunu tersine çevir (Toggle LED)
        // ODR (Output Data Register) üzerinden 5. biti XOR ile tersliyoruz.
        GPIOA->ODR ^= (1 << 5);

        // 4. Basit bir gecikme (Dummy delay)
        // Henüz timer kurmadığımız için işlemciyi oyalayan basit bir for döngüsü.
        // Optimizasyonun bu döngüyü silmemesi için 'volatile' kullanıyoruz.
        for (volatile int i = 0; i < 500000; i++) {
            // Boş bekleme
        }
    }
}
