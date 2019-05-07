//
// Created by Apple on 2019/4/9.
//


#include "../Utils/Includes.h"
#include "Mono.h"

const char *save_path = "/sdcard/myhook/U3D/";

void (*old_mono_image_init)(MonoImage *image) = NULL;
void new_mono_image_init(MonoImage *image)
{
    old_mono_image_init(image);

    DUALLOGD("[dumpdll] [%s] name[%s] point[%p] len[%d]", __FUNCTION__, image->name, image->raw_data, image->raw_data_len);

    const char *G_packageName = G_HookConfig->pack_name.c_str();
    if (image->name != NULL)
        dump_write(G_packageName, save_path, image->name, image->raw_data, (size_t)image->raw_data_len);
}

void unity_entry(void *handle)
{
    if (G_HookConfig->dump_dll)
    {
        //修改hook函数为mono_image_init针对腾讯游戏如：飞车，mono_image_open_from_data_with_name函数hook崩溃
        MS(handle, "mono_image_init", mono_image_init);
    }
}