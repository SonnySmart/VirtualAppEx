//
// Created by Apple on 2019/5/5.
//

#include "../Utils/Includes.h"
#include <sys/mman.h>

void unshell_so_entry(const char *name, void *handle)
{
    FILE *fp = NULL;
    char line[1024] = { 0 };

    char *pch;
    long beginAddr;
    long endAddr;
    std::vector<std::string> strLine;
    std::vector<std::string>::iterator it;
    int length = 0;

    if ((fp = fopen("/proc/self/maps", "r")) == NULL) {
        DUALLOGE("[-] [%s] [%s]", __FUNCTION__, errno);
        return;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, HOOK_NAME))
        {
            strLine.push_back(line);
            DUALLOGD("unshell_so_entry[%s]", line);
        }
    }

    fclose(fp);

    it = strLine.begin();
    char *new_line = (char *) malloc(1024);
    char *new_line2 = (char *) malloc(1024);
    strcpy(new_line, (*it).c_str());
    pch = strtok(new_line, "-");
    char* begAddr=pch;
    if (pch != NULL) {
        beginAddr = strtoul(pch, NULL, 16);
    }
    it = strLine.end() - 1;
    strcpy(new_line2, (*it).c_str());
    pch = strtok(new_line2, " ");
    if (pch != NULL) {
        pch = strtok(pch, "-");
        pch = strtok(NULL, "-");
        DUALLOGE("pch--%s", pch);
    }
    endAddr = strtoul(pch, NULL, 16);
    DUALLOGE("beginAddr=%ld", beginAddr);
    DUALLOGE("endAddr=%ld", endAddr);
    length = (int) (endAddr - beginAddr);
    free(new_line);
    free(new_line2);
    new_line = NULL;
    new_line2 = NULL;
    uint32_t page_size = getpagesize();
    int n = length / page_size;
    int ret = mprotect((void *) beginAddr, page_size * n, PROT_READ | PROT_WRITE | PROT_EXEC);
    DUALLOGE("page_size=%d", page_size);
    DUALLOGE("length=%d", length);
    char *memory = (char *) malloc(length * sizeof(char));
    memcpy(memory, (void *) beginAddr, length);

    dump_write(PACK_NAME, ASSET_PATH, HOOK_NAME, memory, length);
}