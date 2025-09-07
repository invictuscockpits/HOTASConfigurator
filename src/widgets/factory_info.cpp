#include "factory_info.h"
#include <string.h>

static void sanitize_and_copy(char *dst, const char *src, size_t maxlen)
{
    size_t i = 0;
    for (; i + 1 < maxlen && src[i]; ++i) {
        unsigned char c = (unsigned char)src[i];
        dst[i] = (c >= 0x20 && c <= 0x7E) ? (char)c : '_'; // printable ASCII only
    }
    dst[i] = '\0';
    for (++i; i < maxlen; ++i) dst[i] = 0; // deterministic CRC
}

v

force_factory_anchors_t g_factory;
static int dom_is_valid_ascii(const char* dom10)
{
    /* YYYY-MM-DD */
    for (int i = 0; i < DOM_ASCII_LEN; ++i) {
        unsigned char c = (unsigned char)dom10[i];
        if (i == 4 || i == 7) { if (c != '-') return 0; }
        else { if (!isdigit(c)) return 0; }
    }
    return 1;
}

void factory_info_handle_set_dom(const char dom[DOM_ASCII_LEN])
{
    if (!dom) return;
    if (g_factory.lock_bits & LOCKBIT_DOM) return;           /* locked */
    if (!dom_is_valid_ascii(dom)) return;

    /* Save into reserved[0..9] */
    if (sizeof(g_factory.reserved) >= DOM_ASCII_LEN) {
        memcpy(g_factory.reserved, dom, DOM_ASCII_LEN);
        /* zero the tail so CRC is stable, if you care */
        for (size_t i = DOM_ASCII_LEN; i < sizeof(g_factory.reserved); ++i)
            g_factory.reserved[i] = 0;
        if (g_factory.version < 2) g_factory.version = 2;
        factory_store_save(&g_factory);
    }
}

uint16_t factory_info_build_identity_payload(uint8_t* out, uint16_t outCap)
{
    if (!out || outCap == 0) return 0;

    /* layout: model[INV_MODEL_MAX_LEN], serial[INV_SERIAL_MAX_LEN], optional 10B DoM */
    const uint16_t need = (uint16_t)(INV_MODEL_MAX_LEN + INV_SERIAL_MAX_LEN);
    uint16_t wr = 0;
    if (outCap < need) return 0;

    /* model (NUL-terminated in anchors; copy full fixed-length window, keep trailing NULs) */
    memcpy(out + wr, g_factory.device_model, INV_MODEL_MAX_LEN); wr += INV_MODEL_MAX_LEN;

    /* serial */
    memcpy(out + wr, g_factory.device_serial, INV_SERIAL_MAX_LEN); wr += INV_SERIAL_MAX_LEN;

    /* optional DoM */
    if (sizeof(g_factory.reserved) >= DOM_ASCII_LEN &&
        dom_is_valid_ascii((const char*)g_factory.reserved)) {
        if (outCap - wr >= DOM_ASCII_LEN) {
            memcpy(out + wr, g_factory.reserved, DOM_ASCII_LEN);
            wr += DOM_ASCII_LEN;
        }
    }
    return wr;
}
// TEMPORARY: satisfy linker; wire this toflash writer.
#if defined(__GNUC__)
__attribute__((weak))
#endif
void factory_store_save(const force_factory_anchors_t* /*f*/)
{
    /* TODO: call your real anchors flash-writer here so identity persists.
       For now this is a no-op so we can keep moving and test end-to-end. */
}
