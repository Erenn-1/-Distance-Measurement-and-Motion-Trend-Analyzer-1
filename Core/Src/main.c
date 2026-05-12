#include "main.h"

/*
 * Project 2 – Distance Measurement and Motion Trend Analyzer
 * Platform  : STM32 NUCLEO-F411RE, bare-metal (no HAL)
 * Sensor    : HC-SR04 ultrasonic distance sensor
 * Peripherals used: GPIO, TIM2 (timer-based pulse generation & timing), USART2
 *
 * Hardware wiring:
 *   HC-SR04 TRIG  -->  PA5  (GPIO output, 10 µs pulse)
 *   HC-SR04 ECHO  -->  PA0  (GPIO input, echo width measured via TIM2)
 *   USART2  TX    -->  PA2  (AF7, connected to ST-Link VCP)
 *   USART2  RX    -->  PA3  (AF7, connected to ST-Link VCP)
 *   VCC: 5 V, GND: GND
 *
 * UART settings: 115200 baud, 8N1. Open any serial terminal.
 *
 * Modes (select by sending a character):
 *   R - Reference Mode : 10 samples at each of 5 fixed target positions
 *   D - Dynamic Mode   : continuous measurement with motion trend + event logging
 *   S - Single shot    : one distance reading
 */

/* ====== Sabitler (Configuration) ======
 * Tüm sayısal parametreler burada toplandı; kodun içine gömülü "sihirli sayı" yok */
#define REF_POSITIONS        5U    /* Referans modunda kaç farklı konum ölçülecek */
#define REF_SAMPLES_PER_POS  10U   /* Her konumdan kaç örnek alınacak */
#define SAMPLE_INTERVAL_MS   100U  /* Örnekler arası bekleme → 10 Hz örnekleme hızı */
#define NEAR_THRESHOLD_CM    20U   /* Bu değerin altına giren nesne "yakın olay" sayılır */
#define FAR_THRESHOLD_CM     100U  /* Bu değerin üstüne çıkan nesne "uzak olay" sayılır */
#define DYNAMIC_SAMPLES      200U  /* Dynamic Mode en fazla bu kadar örnek alır, sonra menüye döner */

/* ====== İleri bildirimler (Forward declarations) ======
 * Bu fonksiyonlar main'den sonra tanımlandığı için derleyiciye önden haber veriliyor */
static void run_reference_mode(void);
static void run_dynamic_mode(void);
static void print_banner(void);
static void print_stats(uint32_t *buf, uint32_t n);

/* =========================================================
 * main: Programın giriş noktası.
 * Sırasıyla donanımı başlatır, karşılama ekranını gösterir,
 * ardından sonsuz döngüde kullanıcı komutu bekler.
 * ========================================================= */
int main(void) {

    /* Saat, SysTick ve TIM2'yi başlat — diğer her şey buna bağımlı */
    system_init();

    /* USART2 pinlerini ve baud rate'i ayarla */
    uart_init();

    /* HC-SR04 için TRIG (PA5 çıkış) ve ECHO (PA0 giriş) pinlerini ayarla */
    hc_sr04_init();

    /* Terminal ekranına başlık ve kablo bağlantı bilgilerini yaz */
    print_banner();

    /* Ana menü döngüsü: kullanıcı R / D / S gönderene kadar bekle */
    while (1) {
        uart_print("\r\n> Send  R  D  or  S  : ");

        /* Karakter gelene kadar blokla (timeout=0 → sonsuza kadar bekle) */
        char cmd = uart_getchar_wait(0);

        switch (cmd) {
            /* R veya r → Referans modunu başlat */
            case 'R': case 'r': run_reference_mode(); break;

            /* D veya d → Dinamik modu başlat */
            case 'D': case 'd': run_dynamic_mode();   break;

            /* S veya s → Tek seferlik anlık ölçüm yap */
            case 'S': case 's': {
                uint32_t d = hc_sr04_measure_cm_x100();
                uart_print("\r\nDistance: ");
                /* 0 dönüşü → sensör timeout (nesne bulunamadı veya çok uzak) */
                if (d == 0U) { uart_print("OUT OF RANGE\r\n"); }
                else         { uart_print_cm(d); uart_print(" cm\r\n"); }
                break;
            }
            /* Tanımsız tuş → yoksay, menüye dön */
            default: break;
        }
    }
}

/* =========================================================
 * run_reference_mode: Kullanıcı sabit mesafelere nesne koyar,
 * her konumda 10 ölçüm alınır, min/max/avg/gürültü yazdırılır.
 * 5 konum × 10 örnek = 50 ölçüm.
 * ========================================================= */
static void run_reference_mode(void) {

    /* Geçerli örnekleri tutmak için yerel dizi (timeout olanlar dahil edilmez) */
    uint32_t samples[REF_SAMPLES_PER_POS];

    /* Başlık ekranını yazdır */
    uart_print("\r\n");
    uart_print("========================================\r\n");
    uart_print("        REFERENCE MODE\r\n");
    uart_print("========================================\r\n");
    uart_print("Place target at a fixed distance.\r\n");
    uart_print("Press any key to record 10 samples,\r\n");
    uart_print("then move to the next position.\r\n\r\n");

    /* 5 konum için döngü */
    for (uint32_t pos = 1U; pos <= REF_POSITIONS; pos++) {

        /* Kullanıcı nesneyi yerleştirip hazır olunca herhangi bir tuşa bassın */
        uart_print("Position ");
        uart_print_uint(pos);
        uart_print(" -> Press any key when ready... ");
        uart_getchar_wait(0);  /* Tuşa basılana kadar bekle */
        uart_print("\r\n");

        uint32_t valid = 0U;  /* Bu konumdaki geçerli (timeout olmayan) örnek sayısı */

        /* 10 örnek al */
        for (uint32_t i = 0U; i < REF_SAMPLES_PER_POS; i++) {

            /* HC-SR04 datasheet: ölçümler arası en az 60 ms önerilir, 200 ms tercih edildi */
            delay_ms(200U);

            uint32_t d = hc_sr04_measure_cm_x100();

            /* Örnek numarasını yazdır */
            uart_print("  [");
            uart_print_uint(i + 1U);
            uart_print("] ");

            if (d == 0U) {
                /* Sensör yanıt vermedi → timeout, bu örneği istatistiğe katma */
                uart_print("TIMEOUT\r\n");
            } else {
                /* Geçerli ölçüm → ekrana yaz ve diziye kaydet */
                uart_print_cm(d);
                uart_print(" cm\r\n");
                samples[valid++] = d;  /* valid indeksini artırarak diziye ekle */
            }
        }

        /* En az 1 geçerli örnek varsa istatistik hesapla */
        if (valid > 0U) {
            uart_print("  --- Stats ---\r\n");
            print_stats(samples, valid);
        }
        uart_print("\r\n");
    }

    uart_print("Reference mode complete. Returning to menu.\r\n");
}

/* =========================================================
 * print_stats: Verilen cm_x100 dizisi için
 * ortalama, minimum, maksimum ve gürültü (max-min) hesaplayıp yazdırır.
 * ========================================================= */
static void print_stats(uint32_t *buf, uint32_t n) {

    /* mn başlangıçta maksimum uint32 değeri → ilk karşılaştırmada gerçek min olur */
    uint32_t sum = 0U, mn = 0xFFFFFFFFU, mx = 0U;

    for (uint32_t i = 0U; i < n; i++) {
        sum += buf[i];                    /* Toplam (ortalama için) */
        if (buf[i] < mn) mn = buf[i];    /* Minimum güncelle */
        if (buf[i] > mx) mx = buf[i];    /* Maksimum güncelle */
    }

    /* Tamsayı bölmesi: ondalık kısmı korumak için değerler zaten *100 formatında */
    uint32_t avg = sum / n;

    uart_print("  AVG: "); uart_print_cm(avg);      uart_print(" cm\r\n");
    uart_print("  MIN: "); uart_print_cm(mn);        uart_print(" cm\r\n");
    uart_print("  MAX: "); uart_print_cm(mx);        uart_print(" cm\r\n");

    /* Gürültü = maksimum sapma; sensörün tekrarlanabilirliğini gösterir */
    uart_print("  NOISE (max-min): "); uart_print_cm(mx - mn); uart_print(" cm\r\n");
}

/* =========================================================
 * run_dynamic_mode: Sürekli ölçüm yaparak her satırda
 * zaman damgası, mesafe, trend, anlık hız, ortalama hız,
 * yakın/uzak olay sayısını CSV formatında UART'a yazar.
 *
 * 200 örnek sonunda veya 'Q' tuşuna basılınca menüye döner.
 * ========================================================= */
static void run_dynamic_mode(void) {

    /* Bir önceki örneğin mesafesi (trend ve hız hesabı için) */
    uint32_t prev_cm_x100 = 0U;

    /* Kenar tetiklemeli olay sayaçları */
    uint32_t near_events  = 0U;  /* Yakın bölgeye giriş sayısı (< 20 cm) */
    uint32_t far_events   = 0U;  /* Uzak bölgeye giriş sayısı (> 100 cm) */

    /* Nesnenin şu an ilgili bölgede olup olmadığını takip eder
     * (her içeride kalışta sayaç artmasın, sadece girişte artsın) */
    uint8_t  in_near      = 0U;
    uint8_t  in_far       = 0U;

    /* Kayan pencere tamponunu temizle */
    motion_init();

    /* Başlık ve format bilgisi */
    uart_print("\r\n");
    uart_print("========================================\r\n");
    uart_print("        DYNAMIC MODE\r\n");
    uart_print("========================================\r\n");
    uart_print("Format: [ts_ms], dist_cm, TREND, spd_cm/s, avg_spd_cm/s, near_ev, far_ev\r\n");
    uart_print("Send 'Q' to stop early.\r\n\r\n");

    /* 200 örnek döngüsü */
    for (uint32_t sample = 0U; sample < DYNAMIC_SAMPLES; sample++) {

        /* Bloklamayan okuma: Q gelirse döngüden çık (program beklemez) */
        int8_t ch = uart_getchar_nb();
        if (ch == 'Q' || ch == 'q') break;

        /* Zaman damgasını ölçümden ÖNCE al → gecikme zaman farkını etkilemesin */
        uint32_t ts      = get_tick_ms();
        uint32_t cm_x100 = hc_sr04_measure_cm_x100();

        /* Sensör yanıt vermediyse bu örneği atla */
        if (cm_x100 == 0U) {
            uart_print("[");
            uart_print_uint(ts);
            uart_print("] OUT_OF_RANGE\r\n");
            delay_ms(SAMPLE_INTERVAL_MS);
            continue;  /* for döngüsünün başına dön */
        }

        /* İlk örnekte önceki değer yok → trend ve hız hesaplanamaz */
        motion_trend_t trend   = STATIONARY;
        uint32_t       spd     = 0U;
        uint32_t       avg_spd = 0U;

        if (sample > 0U && prev_cm_x100 != 0U) {
            /* Yaklaşıyor / Uzaklaşıyor / Sabit kararını ver */
            trend = motion_classify(prev_cm_x100, cm_x100);

            /* Anlık hız: iki örnek arası mesafe / zaman (cm/s * 100) */
            spd   = motion_speed_x100(prev_cm_x100, cm_x100, SAMPLE_INTERVAL_MS);
        }

        /* Yeni örneği kayan pencere tamponuna ekle */
        motion_update_window(cm_x100, ts);

        /* Son 5 örneğin ortalama hızını hesapla */
        avg_spd = motion_avg_speed_x100();

        /* ---- Kenar tetiklemeli olay tespiti ----
         * Nesne bölgeye HER GİRİŞTE bir kez sayılır,
         * içeride kaldığı süre boyunca tekrar sayılmaz */
        uint8_t is_near = (cm_x100 < (NEAR_THRESHOLD_CM * 100U)) ? 1U : 0U;
        uint8_t is_far  = (cm_x100 > (FAR_THRESHOLD_CM  * 100U)) ? 1U : 0U;

        /* Önceki örnekte dışarıdaydı, şimdi içerideyse → yeni giriş olayı */
        if (is_near && !in_near) near_events++;
        if (is_far  && !in_far)  far_events++;

        /* Mevcut durumu bir sonraki örnek için sakla */
        in_near = is_near;
        in_far  = is_far;

        /* ---- CSV formatında UART'a yaz ----
         * Örnek çıktı: [1250], 23.47, APPROACHING, 5.20, 4.80, near=1, far=0 */
        uart_putchar('[');
        uart_print_uint(ts);          /* Zaman damgası (ms) */
        uart_print("], ");
        uart_print_cm(cm_x100);       /* Mesafe (cm.cc formatında) */
        uart_print(", ");
        uart_print(motion_trend_str(trend));  /* APPROACHING / RETREATING / STATIONARY */
        uart_print(", ");
        uart_print_cm(spd);           /* Anlık hız (cm/s) */
        uart_print(", ");
        uart_print_cm(avg_spd);       /* Kayan pencere ortalama hızı (cm/s) */
        uart_print(", near=");
        uart_print_uint(near_events); /* Toplam yakın bölge giriş sayısı */
        uart_print(", far=");
        uart_print_uint(far_events);  /* Toplam uzak bölge giriş sayısı */
        uart_print("\r\n");

        /* Bir sonraki örnek için mevcut mesafeyi kaydet */
        prev_cm_x100 = cm_x100;

        /* 100 ms bekle → 10 Hz örnekleme hızı */
        delay_ms(SAMPLE_INTERVAL_MS);
    }

    /* Oturum özeti */
    uart_print("\r\n--- Session Summary ---\r\n");
    uart_print("Near events (entered <");
    uart_print_uint(NEAR_THRESHOLD_CM);
    uart_print(" cm zone): ");
    uart_print_uint(near_events);
    uart_print("\r\nFar  events (entered >");
    uart_print_uint(FAR_THRESHOLD_CM);
    uart_print(" cm zone): ");
    uart_print_uint(far_events);
    uart_print("\r\nReturning to menu.\r\n");
}

/* =========================================================
 * print_banner: Program açılışında terminal ekranına
 * proje adı, platform, kablo bağlantıları ve komut listesini yazar.
 * ========================================================= */
static void print_banner(void) {
    uart_print("\r\n");
    uart_print("========================================\r\n");
    uart_print("  Distance Measurement & Motion Analyzer\r\n");
    uart_print("  STM32 NUCLEO-F411RE | Bare-Metal\r\n");
    uart_print("  HC-SR04 | GPIO + TIM2 + USART2\r\n");
    uart_print("========================================\r\n");
    uart_print("Wiring:\r\n");
    uart_print("  HC-SR04 TRIG -> PA5\r\n");
    uart_print("  HC-SR04 ECHO -> PA0\r\n");
    uart_print("  UART TX/RX   -> PA2/PA3 (ST-Link VCP)\r\n\r\n");
    uart_print("Commands:\r\n");
    uart_print("  R - Reference Mode (10 samples x 5 positions)\r\n");
    uart_print("  D - Dynamic Mode   (motion trend + events)\r\n");
    uart_print("  S - Single shot measurement\r\n");
}
