
#include "../Utils/Includes.h"
#include <pthread.h>

using namespace neb;

JavaVM *G_VM = NULL;
ptr_hook_config G_HookConfig = NULL;
ptr_WInlineHookFunction G_WInlineHookFunction = NULL;

// extern
extern "C" bool loadConfig();
extern "C" void hook_entry(const char *name, void *handle);

//extern "C" __attribute__((constructor)) void _start(void) {
//    void *handle = NULL;
//    void *symbol = NULL;
//    do {
//        if (G_VM) break;
//
//        if (!loadConfig()) break;
//
//#if WHALE
//        if (!(handle = dlopen_compat("libwhale.so", RTLD_NOW)))
//            break;
//
//        if (!(symbol = dlsym_compat(handle, "WInlineHookFunction")))
//            break;
//
//        G_WInlineHookFunction = (ptr_WInlineHookFunction)symbol;
//#endif
//    } while (0);
//}

HOOK_DEF(void, onSoLoaded, const char *name, void *handle)
{
    hook_entry(name, handle);
    //OLD_FUNC(onSoLoaded)(name, handle);
}

//注入后初始化并读取配置
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    void *handle = NULL;
    void *symbol = NULL;
    JNIEnv* env = NULL;

    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK)
    {
        DUALLOGE("[-] [%s] GetEnv Error .", __FUNCTION__);
        return JNI_ERR;
    }

	do {
	    if (G_VM) break;

        if (!loadConfig()) break;

#if WHALE
        if (!(handle = dlopen_compat("libwhale.so", RTLD_NOW)))
            break;

        if (!(symbol = dlsym_compat(handle, "WInlineHookFunction")))
            break;

        G_WInlineHookFunction = (ptr_WInlineHookFunction)symbol;
#endif

        if (!(handle = dlopen_compat("libva++.so", RTLD_NOW)))
            break;

        MS(handle, "_Z10onSoLoadedPKcPv", onSoLoaded);
	} while (0);

    G_VM = vm;

    return JNI_VERSION_1_4;
}

// 读取json配置文件
extern "C" bool loadConfig()
{
	std::string jsonString;
	read_string("/sdcard/myhook/config.json", jsonString);
	if (jsonString.empty())
        return false;

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

    return true;
}

jobject getGlobalContext(JNIEnv *env)
{
    //获取Activity Thread的实例对象
    jclass activityThread = env->FindClass("android/app/ActivityThread");
    jmethodID currentActivityThread = env->GetStaticMethodID(activityThread, "currentActivityThread", "()Landroid/app/ActivityThread;");
    jobject at = env->CallStaticObjectMethod(activityThread, currentActivityThread);
    //获取Application，也就是全局的Context
    jmethodID getApplication = env->GetMethodID(activityThread, "getApplication", "()Landroid/app/Application;");
    jobject context = env->CallObjectMethod(at, getApplication);
    env->DeleteLocalRef(activityThread);
    return context;
}

void toast(const char *msg)
{
    JNIEnv *env  =  NULL;
    jint attach = -1;
    jclass clazz = NULL;
    jmethodID mid = NULL;
    jmethodID showId = NULL;
    jstring js = NULL;
    jobject job = NULL;
    jobject context = NULL;

    if (G_VM == NULL)
        return;

    int status = G_VM->GetEnv((void **) &env, JNI_VERSION_1_4);
    if (status < 0)
    {
        attach = G_VM->AttachCurrentThread(&env, NULL);
        //DUALLOGE("[-] attach[%d]", attach);
    }

    if (env == NULL)
    {
        DUALLOGE("[-] [%s] env null", __FUNCTION__);
        goto detach;
    }

    if ((clazz = env->FindClass("android/widget/Toast")) == NULL) {
        DUALLOGE("[-] [%s] clazz null", __FUNCTION__);
        goto detach;
    }

    if ((mid = env->GetStaticMethodID( clazz, "makeText", "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;")) == NULL) {
        DUALLOGE("[-] [%s] mid null", __FUNCTION__);
        goto detach;
    }

    context = getGlobalContext(env);
    if (context == NULL) {
        DUALLOGE("[-] [%s] context null", __FUNCTION__);
        goto detach;
    }
    js = env->NewStringUTF(msg);
    job = env->CallStaticObjectMethod(clazz, mid, context, js, 0);
    showId = env->GetMethodID(clazz,"show","()V");
    env->CallVoidMethod(job,showId);

    env->DeleteLocalRef(js);

    detach:
    if (attach == JNI_OK) G_VM->DetachCurrentThread();
}