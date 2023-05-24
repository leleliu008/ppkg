#include "core/sysinfo.h"
#include "ppkg.h"

int ppkg_sysinfo() {
    SysInfo sysinfo = {0};

    int ret = sysinfo_make(&sysinfo);

    if (ret != PPKG_OK) {
        return ret;
    }

    sysinfo_dump(sysinfo);
    sysinfo_free(sysinfo);
   
    return PPKG_OK;
}
