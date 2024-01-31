/* *******************************************************************************
 * Copyright (c) 2014-2023 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy of
 * the License at:
 *
 *            http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an “AS IS” BASIS, without warranties or
 * conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the License for the
 * specific language governing permissions and limitations under the License.
 * *********************************************************************************/

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "block.h"
#include "diskinfo.h"
#include "parse_cmd.h"

typedef struct {
    DiskInfo hdr;
    block *b;
    uint64_t capacity;
} FlatDiskInfo;

static inline FlatDiskInfo *getFDI(DiskInfo *self) {
    return (FlatDiskInfo *)self;
}

static off_t FlatGetCapacity(DiskInfo *self) {
    FlatDiskInfo *fdi = getFDI(self);

    return fdi->capacity;
}

static ssize_t FlatPread(DiskInfo *self, void *buf, size_t len, off_t pos) {
    FlatDiskInfo *fdi = getFDI(self);
    return fdi->b->pread(fdi->b->opaque, buf, len, pos);
}

static ssize_t FlatPwrite(DiskInfo *self, const void *buf, size_t len, off_t pos) {
    FlatDiskInfo *fdi = getFDI(self);

    /*
     * Should we do some zero detection here to generate sparse file?
     */
    return fdi->b->pwrite(fdi->b->opaque, buf, len, pos);
}

static int FlatClose(DiskInfo *self) {
    FlatDiskInfo *fdi = getFDI(self);
    int ret = fdi->b->close(fdi->b->opaque);
    free(fdi->b);

    free(fdi);
    return ret;
}

static int FlatNextData(DiskInfo *self, off_t *pos, off_t *end) {
    FlatDiskInfo *fdi = getFDI(self);
    off_t dataOff = fdi->b->seek(fdi->b->opaque, *end, SEEK_DATA);
    off_t holeOff;

    if (dataOff == -1) {
        if (errno == ENXIO) {
            return -1;
        }
        dataOff = *end;
        holeOff = fdi->capacity;
        if (dataOff >= holeOff) {
            errno = ENXIO;
            return -1;
        }
    } else {
        holeOff = fdi->b->seek(fdi->b->opaque, dataOff, SEEK_HOLE);
        if (holeOff == -1) {
            holeOff = fdi->capacity;
        }
    }
    *pos = dataOff;
    *end = holeOff;
    return 0;
}

static DiskInfoVMT flatDiskInfoVMT = {.getCapacity = FlatGetCapacity,
                                      .pread = FlatPread,
                                      .pwrite = FlatPwrite,
                                      .nextData = FlatNextData,
                                      .close = FlatClose,
                                      .abort = FlatClose};

DiskInfo *Flat_Open(const char *fileName) {
    block *b = new_zbs_block(args.src_ip, args.src_volume_uuid, O_RDONLY);
    struct stat stb;
    FlatDiskInfo *fdi;

    if (b == NULL) {
        return NULL;
    }

    fdi = malloc(sizeof *fdi);
    if (!fdi) {
        goto errClose;
    }
    fdi->hdr.vmt = &flatDiskInfoVMT;
    fdi->b = b;
    fdi->capacity = b->get_size(b->opaque);
    return &fdi->hdr;
errClose:
    b->close(b->opaque);
    return NULL;
}

DiskInfo *Flat_Create(const char *fileName, off_t capacity) {
    block *b = new_zbs_block(args.dest_ip, args.dest_volume_uuid, 0);
    FlatDiskInfo *fdi;

    if (b == NULL) {
        return NULL;
    }
    fdi = malloc(sizeof *fdi);
    if (!fdi) {
        goto errClose;
    }
    fdi->hdr.vmt = &flatDiskInfoVMT;
    fdi->b = b;
    fdi->capacity = capacity;
    return &fdi->hdr;
errClose:
    b->close(b->opaque);
    return NULL;
}
