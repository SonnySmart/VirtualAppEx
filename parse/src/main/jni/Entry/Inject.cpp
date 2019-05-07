
#include "../Utils/Includes.h"
#include <pthread.h>

using namespace neb;

JavaVM *G_VM = NULL;
ptr_hook_config G_HookConfig = NULL;

// self extern
extern "C" void hook_entry(const char *name, void *handle);

// 读取json配置文件
void loadConfig()
{
	std::string jsonString = read_string("/sdcard/myhook/config.json");
	if (jsonString.empty())
	{
		DUALLOGE("[-] /sdcard/myhook/config.json failed!\n");
		return;
	}

	std::string dump_lua;
	std::string dump_dll;
	std::string dump_res;
    std::string dump_res1;
    std::string dump_xxtea;
	G_HookConfig = new hook_config();
	CJsonObject *json = new CJsonObject(jsonString);
	json->Get("pack_name", G_HookConfig->pack_name);
	json->Get("hook_name", G_HookConfig->hook_name);
	json->Get("dump_lua", dump_lua);
	json->Get("dump_dll", dump_dll);
	json->Get("dump_res", dump_res);
    json->Get("dump_res1", dump_res1);
    json->Get("dump_xxtea", dump_xxtea);
	delete json;

	G_HookConfig->dump_lua = dump_lua == "1";
	G_HookConfig->dump_dll = dump_dll == "1";
	G_HookConfig->dump_res = dump_res == "1";
    G_HookConfig->dump_res1 = dump_res1 == "1";
    G_HookConfig->dump_xxtea = dump_xxtea == "1";
}

HOOK_DEF(void, onSoLoaded, const char *name, void *handle)
{
    hook_entry(name, handle);
    //OLD_FUNC(onSoLoaded)(name, handle);
}

//注入后初始化并读取配置
extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env = NULL;
	if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK)
	{
		DUALLOGE("[-] [%s] GetEnv Error .", __FUNCTION__);
		return JNI_ERR;
	}

	if (!G_VM)
    {
        loadConfig();
        if (!G_HookConfig->hook_name.empty())
        {
            void *handle = dlopen_compat("libva++.so", RTLD_NOW);
            if (handle)
            {
                MS(handle, "_Z10onSoLoadedPKcPv", onSoLoaded);
                dlclose_compat(handle);
            }
        }
    }

    G_VM = vm;

	return JNI_VERSION_1_4;
}