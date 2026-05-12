#include "motion.h"
/* Hareket olarak sayılabilmesi için gereken minimum mesafe değişimi: 0.5 cm
 * Bunun altındaki değişimler sensör gürültüsü kabul edilir, STATIONARY döner */
#define MOTION_THRESHOLD_CM_X100  50U

/* Kayan pencere (sliding window) kaç örnek tutacak.
 * Son 5 ölçümün ortalamasından hız hesaplanır */
#define SPEED_WINDOW_SIZE         5U

/* Her pencere elemanı: o anki mesafe + zaman damgası */
typedef struct {
    uint32_t dist_cm_x100; /* Mesafe (cm * 100 formatında, örn. 2347 = 23.47 cm) */
    uint32_t ts_ms;        /* Ölçümün alındığı zaman (milisaniye) */
} speed_sample_t;

/* Dairesel tampon (circular buffer): SPEED_WINDOW_SIZE elemanlı sabit dizi */
static speed_sample_t speed_buf[SPEED_WINDOW_SIZE];

/* Bir sonraki yazılacak yuvayı gösterir (0–4 arası döner) */
static uint8_t speed_idx;

/* Tampon ilk kez dolduğunda 1 olur; dolmadan ortalama hesaplanmaz */
static uint8_t speed_full;

/* =========================================================
 * motion_init: Dynamic Mode başlamadan önce tamponu sıfırlar.
 * speed_idx ve speed_full'u temizlemek yeterli; eski veri
 * zaten n<2 koruması sayesinde okunmaz.
 * ========================================================= */
void motion_init(void) {
    speed_idx  = 0;
    speed_full = 0;
}

/* =========================================================
 * motion_classify: İki ardışık ölçüm arasındaki farka bakarak
 * nesnenin yaklaşıp yaklaşmadığına, uzaklaşıp uzaklaşmadığına
 * ya da yerinde durduğuna karar verir.
 * ========================================================= */
motion_trend_t motion_classify(uint32_t prev_cm_x100, uint32_t curr_cm_x100) {

    /* İşaretli çıkarma: mesafe azaldıysa delta negatif (yaklaşıyor),
     * arttıysa pozitif (uzaklaşıyor) */
    int32_t delta = (int32_t)curr_cm_x100 - (int32_t)prev_cm_x100;

    /* Eşik değerinden fazla azaldı → nesne yaklaşıyor */
    if (delta < -(int32_t)MOTION_THRESHOLD_CM_X100) return APPROACHING;

    /* Eşik değerinden fazla arttı → nesne uzaklaşıyor */
    if (delta >  (int32_t)MOTION_THRESHOLD_CM_X100) return RETREATING;

    /* İki eşik arasında kaldı → sensör gürültüsü, sabit say */
    return STATIONARY;
}

/* =========================================================
 * motion_trend_str: Enum değerini UART'a yazdırılacak
 * sabit genişlikli (11 karakter) stringe çevirir.
 * Boşluklar CSV sütun hizalaması içindir.
 * ========================================================= */
const char *motion_trend_str(motion_trend_t t) {
    switch (t) {
        case APPROACHING: return "APPROACHING";
        case RETREATING:  return "RETREATING ";
        default:          return "STATIONARY ";
    }
}

/* =========================================================
 * motion_speed_x100: İki ardışık ölçüm arasındaki anlık hızı
 * hesaplar. Sonuç cm/s * 100 formatındadır
 * (örn. 1523 = 15.23 cm/s).
 * ========================================================= */
uint32_t motion_speed_x100(uint32_t prev_cm_x100, uint32_t curr_cm_x100, uint32_t dt_ms) {

    /* Zaman farkı sıfırsa sonsuz hız çıkar, 0 döndür */
    if (dt_ms == 0U) return 0U;

    /* Mesafe farkını hesapla (işaretli olabilir) */
    int32_t delta = (int32_t)curr_cm_x100 - (int32_t)prev_cm_x100;

    /* Hız yönden bağımsız olduğu için mutlak değer al */
    uint32_t abs_delta = (delta < 0) ? (uint32_t)(-delta) : (uint32_t)delta;

    /* Birim dönüşümü:
     *   abs_delta birimi : cm * 100
     *   dt_ms     birimi : ms
     *
     *   hız (cm/s) = (abs_delta / 100) / (dt_ms / 1000)
     *              = abs_delta * 1000 / (100 * dt_ms)
     *
     *   hız * 100  = abs_delta * 1000 / dt_ms
     *   (bölme en sona bırakılır → tamsayı hassasiyeti korunur) */
    return (abs_delta * 1000U) / dt_ms;
}

/* =========================================================
 * motion_update_window: Her yeni ölçümü dairesel tampona yazar.
 *
 * Tampon dolmadan:  [0][1][2][ ][ ]  speed_full=0
 * Tampon dolduktan: [0][1][2][3][4]  speed_full=1
 * Sonraki yazma en eski yuvayı ezer (dairesel).
 * ========================================================= */
void motion_update_window(uint32_t dist_cm_x100, uint32_t ts_ms) {

    /* Mevcut yuvaya yeni ölçümü yaz */
    speed_buf[speed_idx].dist_cm_x100 = dist_cm_x100;
    speed_buf[speed_idx].ts_ms        = ts_ms;

    /* İndeksi bir ilerlet, sona gelince başa dön (dairesel) */
    speed_idx = (uint8_t)((speed_idx + 1U) % SPEED_WINDOW_SIZE);

    /* İndeks tekrar 0'a döndü → tampon ilk kez tam doldu */
    if (speed_idx == 0U) speed_full = 1;
}

/* =========================================================
 * motion_avg_speed_x100: Tampondaki örneklerden kayan pencere
 * ortalama hızını hesaplar.
 *
 * Yöntem: ardışık örnekler arasındaki mesafe farklarını topla,
 * toplam süreye böl → cm/s * 100 cinsinden ortalama hız.
 * ========================================================= */
uint32_t motion_avg_speed_x100(void) {

    /* Tamponda kaç geçerli örnek var?
     * Doluysa: 5 adet, dolmadıysa: speed_idx adet */
    uint8_t n = speed_full ? SPEED_WINDOW_SIZE : speed_idx;

    /* En az 2 örnek olmadan hız hesaplanamaz */
    if (n < 2U) return 0U;

    /* En eski örneğin indeksini bul:
     * Tampon doluysa → speed_idx (bir sonraki yazılacak = en eski)
     * Tampon dolmadıysa → 0 (her zaman 0'dan başlandı) */
    uint8_t oldest = speed_full ? speed_idx : 0U;

    /* Ardışık örnekler arasındaki toplam mesafe değişimini hesapla */
    uint32_t total_dist = 0U;
    for (uint8_t i = 0U; i < (n - 1U); i++) {

        /* a: i. örnek, b: bir sonraki örnek (dairesel indeks) */
        uint8_t a = (uint8_t)((oldest + i)      % SPEED_WINDOW_SIZE);
        uint8_t b = (uint8_t)((oldest + i + 1U) % SPEED_WINDOW_SIZE);

        /* Mesafe farkı (yön önemli değil, mutlak değer alınır) */
        int32_t d = (int32_t)speed_buf[b].dist_cm_x100
                  - (int32_t)speed_buf[a].dist_cm_x100;
        total_dist += (d < 0) ? (uint32_t)(-d) : (uint32_t)d;
    }

    /* En yeni örneğin indeksini bul:
     * speed_idx her zaman "bir sonraki boş yuva"yı gösterir,
     * bu yüzden en yeni = speed_idx - 1 (sıfırsa son eleman) */
    uint8_t newest = (speed_idx == 0U) ? (SPEED_WINDOW_SIZE - 1U)
                                       : (speed_idx - 1U);

    /* Penceredeki toplam süre (en yeni - en eski zaman damgası) */
    uint32_t dt_ms = speed_buf[newest].ts_ms - speed_buf[oldest].ts_ms;

    /* Süre sıfırsa bölme hatasını önle */
    if (dt_ms == 0U) return 0U;

    /* Ortalama hız hesabı (motion_speed_x100 ile aynı birim dönüşümü):
     *   avg_speed * 100 = total_dist_cm_x100 * 1000 / dt_ms */
0U;

    /* avg_speed_x100 = total_dist_cm_x100 * 1000 / dt_ms */
0U;

    /* avg_speed_x100 = total_dist_cm_x100 * 1000 / dt_ms */
    return (total_dist * 1000U) / dt_ms;
}
