
#include "../Utils/Includes.h"

extern void unity_entry(const char *name, void *handle);
extern void cocos_entry(const char *name, void *handle);

void *symbol = NULL;

// extern
extern "C" void hook_entry(const char *name, void *handle)
{
	if (!handle)
    {
        DUALLOGE("[-] [%s] handle[%p] dlerrpr[%s]", __FUNCTION__, handle, dlerror());
        return;
    }

    DUALLOGD("[+] [%s] name[%s]", __FUNCTION__, name);

    if (!symbol)
    {
        //mono_image_init
        if (findSymbol("mono_image_init", HOOK_NAME, (unsigned long *)&symbol) == 0)
        //if ((symbol = dlsym(handle, "mono_image_init")))
        {
            unity_entry(name, handle);
            DUALLOGD("[+] [%s] name[%s] handle[%p] hooked .", __FUNCTION__, name, handle);
        } else {
            DUALLOGE("[-] [%s] handle[%p] dlerrpr[%s]", __FUNCTION__, handle, dlerror());
        }
        //_ZN7cocos2d14cocos2dVersionEv
        //luaL_loadbufferx
#if 0
        if (1)
#else
        if (findSymbol("_ZN7cocos2d14cocos2dVersionEv", HOOK_NAME, (unsigned long *)&symbol) == 0)
        //if ((symbol = dlsym(handle, "_ZN7cocos2d14cocos2dVersionEv")))
#endif
        {
            cocos_entry(name, handle);
            DUALLOGD("[+] [%s] name[%s] handle[%p] hooked .", __FUNCTION__, name, handle);
        } else {
            DUALLOGE("[-] [%s] handle[%p] dlerrpr[%s]", __FUNCTION__, handle, dlerror());
        }
    }
}