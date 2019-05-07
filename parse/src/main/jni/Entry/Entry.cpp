
#include "../Utils/Includes.h"

extern void unity_entry(void *handle);
extern void cocos_entry(void *handle);
extern void unshell_so_entry(void *base);

void *symbol = NULL;

// extern
extern "C" void hook_entry(const char *name, void *handle)
{
	if (!handle)
    {
        DUALLOGE("[-] [%s] handle[%p]", __FUNCTION__, handle);
        return;
    }

    if (!symbol)
    {
        //mono_image_init
        if ((symbol = dlsym_compat(handle, "mono_image_init")))
        {
            unity_entry(handle);
            goto pass;
        }
        //_ZN7cocos2d14cocos2dVersionEv
        if ((symbol = dlsym_compat(handle, "_ZN7cocos2d14cocos2dVersionEv")))
        {
            cocos_entry(handle);
            goto pass;
        }
    }

    goto end;

    pass:
    DUALLOGE("[+] [%s] name[%s] handle[%p] hooked .", __FUNCTION__, name, handle);
    return;

	end:
    DUALLOGD("[+] [%s] name[%s] handle[%p] hooked .", __FUNCTION__, name, handle);
}