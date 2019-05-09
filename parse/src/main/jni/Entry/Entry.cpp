
#include "../Utils/Includes.h"

extern void unity_entry(const char *name, void *handle);
extern void cocos_entry(const char *name, void *handle);
extern void unshell_so_entry(void *base);

void *symbol = NULL;

// extern
extern "C" void hook_entry(const char *name, void *handle)
{
	if (!handle)
    {
        DUALLOGE("[-] [%s] handle[%p] dlerrpr[%s]", __FUNCTION__, handle, dlerror());
        return;
    }

    if (!symbol)
    {
//        if (findSymbol("mono_image_init", name, (unsigned long *)&symbol) == 0)
//        {
//            unity_entry(name, handle);
//            goto pass;
//        }
//        if (findSymbol("_ZN7cocos2d14cocos2dVersionEv", name, (unsigned long *)&symbol) == 0)
//        {
//            DUALLOGD("zhao dao le ,");
//            cocos_entry(name, handle);
//            goto pass;
//        }
        //mono_image_init
        if ((symbol = dlsym_compat(handle, "mono_image_init")))
        {
            unity_entry(name, handle);
            DUALLOGD("[+] [%s] name[%s] handle[%p] hooked .", __FUNCTION__, name, handle);
        }
        //_ZN7cocos2d14cocos2dVersionEv
        if ((symbol = dlsym_compat(handle, "_ZN7cocos2d14cocos2dVersionEv")))
        {
            cocos_entry(name, handle);
            DUALLOGD("[+] [%s] name[%s] handle[%p] hooked .", __FUNCTION__, name, handle);
        }
    }
}