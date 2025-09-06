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

void factory_info_handle_set_device_id(const cmd_set_device_id_t* cmd)
{
    if (!cmd) return;

    if (!(g_factory.lock_bits & LOCKBIT_MODEL)) {
        sanitize_and_copy(g_factory.device_model,  cmd->device_model,  INV_MODEL_MAX_LEN);
    }
    if (!(g_factory.lock_bits & LOCKBIT_SERIAL)) {
        sanitize_and_copy(g_factory.device_serial, cmd->device_serial, INV_SERIAL_MAX_LEN);
    }

    if (g_factory.version < 2) g_factory.version = 2;
    factory_store_save(&g_factory);
}

void factory_info_handle_lock_device_id(const cmd_lock_device_id_t* cmd)
{
    if (!cmd) return;
    g_factory.lock_bits |= (cmd->lock_mask & (LOCKBIT_MODEL | LOCKBIT_SERIAL));
    if (g_factory.version < 2) g_factory.version = 2;
    factory_store_save(&g_factory);
}

void factory_info_populate_params_report(params_report_t* r)
{
    if (!r) return;
    memcpy(r->device_model,  g_factory.device_model,  INV_MODEL_MAX_LEN);
    memcpy(r->device_serial, g_factory.device_serial, INV_SERIAL_MAX_LEN);
    r->lock_bits = g_factory.lock_bits;
}

force_factory_anchors_t g_factory;

// TEMPORARY: satisfy linker; wire this toflash writer.
#if defined(__GNUC__)
__attribute__((weak))
#endif
void factory_store_save(const force_factory_anchors_t* /*f*/)
{
    /* TODO: call your real anchors flash-writer here so identity persists.
       For now this is a no-op so we can keep moving and test end-to-end. */
}
