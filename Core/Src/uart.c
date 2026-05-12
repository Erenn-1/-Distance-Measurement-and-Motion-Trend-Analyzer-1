#include "uart.h"
#include "stm32f411xx.h"
#include "system.h"

void uart_init(void) {
    /*
     * PA2 = USART2_TX (AF7)
     * PA3 = USART2_RX (AF7)
     */

    /*MODER register'ı pinlerin giriş mi çıkış mı olacağını belirler . 
    Biz 10 (İkilik sistemde 2) yazarak bu pinlere "Siz artık sıradan pin değilsiniz, Alternatif Fonksiyon (AF) modundasınız" diyoruz. */
    GPIOA->MODER &= ~((3U << (2*2)) | (3U << (3*2)));
    GPIOA->MODER |=  ((2U << (2*2)) | (2U << (3*2)));

    /* Set AF7 for PA2 and PA3 in AFR[0] (pins 0-7) */
    /*Pinleri AF moduna aldık ama içeride bir sürü donanım var. AFR (Alternate Function Register) tablosundan bakıp, 
    PA2 ve PA3'ü tam olarak USART2 birimine bağlayan kodun "7" olduğunu buluyoruz ve bunu register'a yazıyoruz .*/
    GPIOA->AFR[0] &= ~((0xFU << (2*4)) | (0xFU << (3*4)));
    GPIOA->AFR[0] |=  ((7U   << (2*4)) | (7U   << (3*4)));

    /*Pin sürücü hızını "Medium Speed" olarak ayarlar. 115200 baud için yeterli,
     gereksiz elektromanyetik gürültüyü önler.*/
    GPIOA->OSPEEDR |= ((2U << (2*2)) | (2U << (3*2)));

    /*
     * BRR for 115200 baud @ fPCLK1 = 16 MHz (HSI, no PLL):
     *   USARTDIV = 16 000 000 / (16 * 115200) = 8.6805...
     *   Mantissa = 8, Fraction = round(0.6805 * 16) = 11
     *   BRR = (8 << 4) | 11 = 0x008B
     */
    /*BRR (Baud Rate Register), konuşma hızımızı belirler. İşlemcimiz 16 MHz. 
    Formülümüz: $16.000.000 / (16 \times 115200) \approx 8.6805$ . Bunun tam kısmı 8, ondalık kısmı ise 11 (0xB) çıkıyor . 
    Bunları birleştirip 0x8B yazıyoruz. Sonra da CR1 register'ından göndericiyi (TE), alıcıyı (RE) ve ana şalteri (UE) açıyoruz . */
    USART2->BRR = 0x008BU;
    /* Enable Transmit En, Receive En, and USART En */
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

/*USART2->SR durum register'ıdır. Oradaki TXE (Transmit Data Register Empty) bayrağı 1 ise 
"Tampon boş, yeni harf yollayabilirsin" demektir . O bayrak 1 olana kadar while döngüsünde bekliyoruz (blokluyoruz). 
Boşaldığı an DR (Data Register) içine harfi fırlatıyoruz.*/
void uart_putchar(char c) {
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = (uint32_t)(uint8_t)c;
}

/*Tek tek harf yollamak yerine, kelimenin sonundaki gizli bitiş karakterine (\0) gelene kadar 
kelimedeki her harfi sırayla yukarıdaki uart_putchar fonksiyonuna yediriyoruz .*/
void uart_print(const char *s) {
    while (*s) uart_putchar(*s++);
}

/*endini çağıran (recursive) bir yapı. Sayı 10'dan büyükse kendini bölerek tekrar çağırıyor. 
Örneğin 123 sayısı için önce en başa gidip 1 rakamını, sonra 2'yi, sonra 3'ü yakalıyor . 
Rakamı karaktere çevirmek için ASCII tablosundaki '0' (48) değeriyle topluyor. Bu çok profesyonel bir bare-metal hilesidir!*/
static void print_digits(uint32_t n) {
    if (n >= 10U) print_digits(n / 10U);
    uart_putchar((char)('0' + (n % 10U)));
}

/*uint → doğrudan print_digits.
int → negatifse önce '-' gönder, sonra mutlak değeri yaz.*/
void uart_print_uint(uint32_t n) {
    print_digits(n);
}

void uart_print_int(int32_t n) {
    if (n < 0) { uart_putchar('-'); n = -n; }
    print_digits((uint32_t)n);
}

/*Ondalıklı cm Yaz*/
/*Sisteme ağır gelen float kullanmamak için mesafeyi 100 ile çarpılmış tam sayı olarak tutmuştuk. 
Sayıyı 100'e bölüp tam kısmını yazdırıyoruz (whole), araya manuel olarak nokta . karakteri koyuyoruz, 
sonra da kalanı (frac) yazdırıyoruz . İşlemci sıfır eforla ondalıklı sayı basmış oluyor.*/
void uart_print_cm(uint32_t cm_x100) {
    uint32_t whole = cm_x100 / 100U;
    uint32_t frac  = cm_x100 % 100U;
    print_digits(whole);
    uart_putchar('.');
    if (frac < 10U) uart_putchar('0');
    print_digits(frac);
}

/*Bloklamayan Karakter Al*/
/*RXNE (Read Data Register Not Empty) bayrağına bakar. 
"Gelen kutusunda karakter var mı?" Varsa hemen o karakteri alır (DR), yoksa asla beklemez ve -1 döndürür . 
Sensör sürekli ölçüm yaparken sistemi dondurmadan araya girip veri okumak için kullanılır.*/
int8_t uart_getchar_nb(void) {
    if (USART2->SR & USART_SR_RXNE)
        return (int8_t)(USART2->DR & 0xFFU);
    return -1;
}

/*Bekleyerek Karakter Al*/
/*Bu bloklayan fonksiyondur. Eğer timeout_ms değerini 0 verirsen, 
RXNE bayrağı 1 olana kadar (yani sen klavyeden tuşa basana kadar) while döngüsünün içinde sistemi tamamen kilitler . 
Ana menüde seçim beklerken bunu kullanıyoruz.*/
char uart_getchar_wait(uint32_t timeout_ms) {
    if (timeout_ms == 0U) {
        while (!(USART2->SR & USART_SR_RXNE));
        return (char)(USART2->DR & 0xFFU);
    }
    uint32_t start = get_tick_ms();
    while (!(USART2->SR & USART_SR_RXNE)) {
        if ((get_tick_ms() - start) >= timeout_ms) return '\0';
    }
    return (char)(USART2->DR & 0xFFU);
}
