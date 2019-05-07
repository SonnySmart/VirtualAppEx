//
// Created by Apple on 2019/5/5.
//

#include "../Utils/Includes.h"

void unshell_so_entry(void *base)
{
    void *addr=(void*)(*(int*)((size_t)base+0x8c));
    uint32_t len=*(uint32_t*)((size_t)base+0x90);
    char filename[128] = { 0 };
    sprintf(filename,"%p_0x%x.so",addr,len);
    dump_write(G_HookConfig->pack_name.c_str(), ASSET_PATH, filename, (const char *)addr, len);
}