#ifndef FORCE_ANCHORS_H
#define FORCE_ANCHORS_H


#include <stdint.h>
#include "common_types.h"     // for force_factory_anchors_t

// CRC over fields excluding crc32; implement in your util module.
uint32_t force_crc32(const void *data, uint32_t len);

// Read/Write/Lock API used by HID handlers (dev-only tab)
bool force_anchors_read(force_factory_anchors_t *out);         // returns false if magic/crc invalid
bool force_anchors_write(const force_factory_anchors_t *in);   // only if !sealed
bool force_anchors_lock(void);                                 // sets sealed=1 and rewrites block

// Helper to validate a block already in RAM (magic+crc)
bool force_anchors_valid(const force_factory_anchors_t *blk);

#endif // FORCE_ANCHORS_H
