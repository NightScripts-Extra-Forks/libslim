/*
Copyright (c) 2020, chyyran
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL CHYYRAN BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * libfat compatibility API
 */

#define ARGV_SUPPORT

#include <sys/iosupport.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>
#include <nds/arm9/dldi.h>

#include <stdlib.h>
#include <string.h>

#include <ff.h>
#include <diskio.h>
#include <slim.h>
#include <ffvolumes.h>

#ifdef ARGV_SUPPORT
#include <nds/system.h>
#include <strings.h>
#include <limits.h>
#endif

#include "charset.h"
#include "cache.h"

bool fatUnmount(const char *mount)
{
    RemoveDevice(mount);
    size_t len = 0;
    TCHAR *m = mbstoucs2(mount, &len);
    if (f_mount(NULL, m, 1) != FR_OK)
    {
        return false;
    }
    return true;
}

bool fatInitDefault(void)
{
    return fatInit(SLIM_CACHE_SIZE, true);
}

bool fatInit(uint32_t cacheSize, bool _configureDefault)
{
    bool sdMounted = false, fatMounted = false;
    fatMounted = fatMountSimple(FF_MNT_FC ":", dldiGetInternal());
    if (isDSiMode())
    {
        sdMounted = fatMountSimple(FF_MNT_SD ":", get_io_dsisd());
    }
    if (_configureDefault)
    {
        configureDefault(sdMounted ? FF_MNT_SD ":" : FF_MNT_FC ":");
    }

    return sdMounted ? sdMounted : fatMounted;
}

void fatGetVolumeLabel(const char *mount, char *label)
{
    int vol = get_vol(mount);
    size_t len = 0;
    if (vol == -1)
        return;
    TCHAR label_buf[255] = {0};
    TCHAR *p = mbstoucs2(mount, &len);
    f_getlabel(p, label_buf, NULL);
    ucs2tombs(label, label_buf);
}

int FAT_getAttr(const char *file)
{
    FILINFO stat;
    size_t len = 0;
    TCHAR *p = mbstoucs2(file, &len);
    if (f_stat(p, &stat) == FR_OK)
    {
        return stat.fattrib;
    }
    return -1;
}

int FAT_setAttr(const char *file, uint8_t attr)
{
    size_t len = 0;
    TCHAR *p = mbstoucs2(file, &len);
    if (f_chmod(p, attr, AM_RDO | AM_SYS | AM_HID) == FR_OK)
    {
        return 0;
    }
    return -1;
}