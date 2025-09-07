#ifndef FACTORY_INFO_H
#define FACTORY_INFO_H

#include <stddef.h>
#include "common_defines.h"
#include "common_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Provided by your existing anchors/flash code */
extern force_factory_anchors_t g_factory;
void factory_store_save(const force_factory_anchors_t* f);  // existing save used for anchors



/* Called when you build params_report_t */
void factory_info_populate_params_report(params_report_t* r);

/* Reads identity payload for CMD_GET_DEVICE_ID into out (model, serial, optional DoM).
 * Returns the number of bytes written to out (<= outCap). */
uint16_t factory_info_build_identity_payload(uint8_t* out, uint16_t outCap);

/* Sets the Date of Manufacture (ASCII "YYYY-MM-DD"). Ignores if locked. */
void factory_info_handle_set_dom(const char dom[DOM_ASCII_LEN]);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* FACTORY_INFO_H */

