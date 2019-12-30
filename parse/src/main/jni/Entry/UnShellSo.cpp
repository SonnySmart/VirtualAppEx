//
// Created by Apple on 2019/5/5.
//

#include "../Utils/Includes.h"
#include <sys/mman.h>

void unshell_so_entry(const char *name, void *handle)
{
    FILE *maps;
    char buff[256];
    char *token = NULL, *buffer = NULL;
    off_t load_addr = 0, end_addr = 0, tmp_addr = 0;
    size_t size = 0, page_size = 0;
    int found = 0;

    maps = fopen("/proc/self/maps", "r");
    if (!maps) {
        DUALLOGE("failed to open maps");
        return;
    }

    while (fgets(buff, sizeof(buff), maps)) {
        if (strstr(buff, HOOK_NAME)) {
            DUALLOGD("%s", buff);
            ++found;

            /* 获取第一个子字符串 */
            token = strtok(buff, " ");
            token = strtok(token, "-");
            tmp_addr = strtoul(token, NULL, 16);
            //if (tmp_addr != 0 && end_addr != 0 && tmp_addr != end_addr) break;
            if (load_addr == 0) load_addr = tmp_addr;
            token = strtok(NULL, "-");
            end_addr = strtoul(token, NULL, 16);
        }
    }

    fclose(maps);

    if (found == 0) {
        DUALLOGE("failed not found [%s]", HOOK_NAME);
        return;
    }

    size = (size_t)(end_addr - load_addr);

    DUALLOGD("load_addr[%x]", load_addr);
    DUALLOGD("end_addr[%x]", end_addr);
    DUALLOGD("size[%d]", size);

    page_size = (size_t)getpagesize();
    if (mprotect((void *) load_addr, page_size * (size / page_size), PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        DUALLOGE("[-] [%s] mprotect[%s]", __FUNCTION__, errno);
        return;
    }

    buffer = (char *)malloc(size);
    memcpy(buffer, (void *)load_addr, size);

    dump_write(PACK_NAME, ASSET_PATH, HOOK_NAME, buffer, size);
}