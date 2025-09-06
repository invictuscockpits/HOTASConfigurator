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

/* Public API for identity (model/serial) */
void factory_info_handle_set_device_id(const cmd_set_device_id_t* cmd);
void factory_info_handle_lock_device_id(const cmd_lock_device_id_t* cmd);

/* Called when you build params_report_t */
void factory_info_populate_params_report(params_report_t* r);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* FACTORY_INFO_H */

