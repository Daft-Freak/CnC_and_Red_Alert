#include "fatfs/ff.h"
#include "fatfs/diskio.h"

#include "storage.h"

// fatfs/storage glue
// guess where this is from...

static bool initialised = false;

// fatfs io funcs
DSTATUS disk_initialize(BYTE pdrv) {
  initialised = storage_init();
  return initialised ? RES_OK : STA_NOINIT;
}

DSTATUS disk_status(BYTE pdrv) {
  return initialised ? RES_OK : STA_NOINIT;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
  static_assert(FF_MIN_SS == FF_MAX_SS);
  return storage_read(sector, 0, buff, FF_MIN_SS * count) == int32_t(FF_MIN_SS * count) ? RES_OK : RES_ERROR;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
  return storage_write(sector, 0, buff, FF_MIN_SS * count) == int32_t(FF_MIN_SS * count) ? RES_OK : RES_ERROR;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
  uint16_t block_size;
  uint32_t num_blocks;

  switch(cmd) {
    case CTRL_SYNC:
      return RES_OK;

    case GET_SECTOR_COUNT:
      get_storage_size(block_size, num_blocks);
      *(LBA_t *)buff = num_blocks;
      return RES_OK;

    case GET_BLOCK_SIZE:
      *(DWORD *)buff = 1;
      return RES_OK;
  }

  return RES_PARERR;
}
