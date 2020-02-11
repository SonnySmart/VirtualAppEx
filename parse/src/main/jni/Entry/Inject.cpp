
#include "../Utils/Includes.h"
#include <pthread.h>

#define FREE(ptr, org_ptr) { if ((void*) ptr != NULL && (void*) ptr != (void*) org_ptr) { free((void*) ptr); } }

using namespace neb;

JavaVM *G_VM = NULL;
ptr_hook_config G_HookConfig = NULL;
ptr_WInlineHookFunction G_WInlineHookFunction = NULL;

// extern
extern "C" bool loadConfig();
extern "C" void hook_entry(const char *name, void *handle);

extern void unshell_so_entry(const char *name, void *handle);

//JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
HOOK_DEF(jint ,JNI_OnLoad, JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK)
    {
        DUALLOGE("[-] [%s] GetEnv Error .", __FUNCTION__);
        return JNI_ERR;
    }
    do {
        if (G_VM) break;

        G_VM = vm;

        //ndk_init(env);
        //InitCrashCaching();
    } while (0);

    jint ret = old_JNI_OnLoad(vm, reserved);

    //unshell_so_entry("", NULL);

    return ret;
}

void onSoLoaded(const char *name, void *handle) {
    ALOGD("[+] [%s] name[%s] handle[%p]", __FUNCTION__, name, handle);

    // so名称包含了包名 && hook名
    if (strstr(name, PACK_NAME) && strstr(name, HOOK_NAME)) {
        //MS(handle, "JNI_OnLoad", JNI_OnLoad);
        hook_entry(name, handle);
    }
}

HOOK_DEF(void*, dlopen, const char *filename, int flag) {
    int res;
    const char *redirect_path = relocate_path(filename, &res);
    void *ret = old_dlopen(redirect_path, flag);
    onSoLoaded(filename, ret);
    //ALOGD("dlopen : %s, return : %p.", redirect_path, ret);
    FREE(redirect_path, filename);
    return ret;
}

HOOK_DEF(void*, do_dlopen_V19, const char *filename, int flag, const void *extinfo) {
    int res;
    const char *redirect_path = relocate_path(filename, &res);
    void *ret = old_do_dlopen_V19(redirect_path, flag, extinfo);
    onSoLoaded(filename, ret);
    //ALOGD("do_dlopen : %s, return : %p.", redirect_path, ret);
    FREE(redirect_path, filename);
    return ret;
}

HOOK_DEF(void*, do_dlopen_V24, const char *name, int flags, const void *extinfo,
         void *caller_addr) {
    int res;
    const char *redirect_path = relocate_path(name, &res);
    void *ret = old_do_dlopen_V24(redirect_path, flags, extinfo, caller_addr);
    onSoLoaded(name, ret);
    //ALOGD("do_dlopen : %s, return : %p.", redirect_path, ret);
    FREE(redirect_path, name);
    return ret;
}

HOOK_DEF(void, onSoLoaded, const char *name, void *handle)
{
    hook_entry(name, handle);
    //OLD_FUNC(onSoLoaded)(name, handle);
}

void hook_dlopen(int api_level) {
    void *symbol = NULL;
    if (api_level > 23) {
        if (findSymbol("__dl__Z9do_dlopenPKciPK17android_dlextinfoPv", "linker",
                       (unsigned long *) &symbol) == 0) {
            DUALLOGD("__dl__Z9do_dlopenPKciPK17android_dlextinfoPv");
            MSHookFunction(symbol, (void *) new_do_dlopen_V24,
                           (void **) &old_do_dlopen_V24);
        }
        else if (findSymbol("__dl__Z9do_dlopenPKciPK17android_dlextinfoPKv", "linker",
                            (unsigned long *) &symbol) == 0) {
            DUALLOGD("__dl__Z9do_dlopenPKciPK17android_dlextinfoPKv");
            MSHookFunction(symbol, (void *) new_do_dlopen_V24,
                           (void **) &old_do_dlopen_V24);
        }
    } else if (api_level >= 19) {
        if (findSymbol("__dl__Z9do_dlopenPKciPK17android_dlextinfo", "linker",
                       (unsigned long *) &symbol) == 0) {
            DUALLOGD("__dl__Z9do_dlopenPKciPK17android_dlextinfo");
            MSHookFunction(symbol, (void *) new_do_dlopen_V19,
                           (void **) &old_do_dlopen_V19);
        }
    } else {
        if (findSymbol("__dl_dlopen", "linker",
                       (unsigned long *) &symbol) == 0) {
            DUALLOGD("__dl_dlopen");
            MSHookFunction(symbol, (void *) new_dlopen, (void **) &old_dlopen);
        }
    }
}

//注入后初始化并读取配置
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;

    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK)
    {
        DUALLOGE("[-] [%s] GetEnv Error .", __FUNCTION__);
        return JNI_ERR;
    }

    DUALLOGD("[+] [%s] sdk_level[%d]", __FUNCTION__, get_sdk_level());

	do {
	    if (G_VM) break;

        //G_VM = vm;

        if (!loadConfig()) break;

        hook_dlopen(get_sdk_level());

#if WHALE
        //libwhale.so
        void *handle = dlopen("libwhale.so", RTLD_LAZY | RTLD_NOW);
        G_WInlineHookFunction = (ptr_WInlineHookFunction)dlsym(handle, "WInlineHookFunction");
        DUALLOGD("G_WInlineHookFunction[%p]", G_WInlineHookFunction);
#endif

        DUALLOGD("[+] [%s] hook_dlopen finish .", __FUNCTION__);
	} while (0);

    return JNI_VERSION_1_6;
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
    std::string dump_res2;
    std::string dump_xxtea;
    std::string dump_inject;
	G_HookConfig = new hook_config();
	CJsonObject *json = new CJsonObject(jsonString);
	json->Get("pack_name", G_HookConfig->pack_name);
	json->Get("hook_name", G_HookConfig->hook_name);
	json->Get("dump_lua", dump_lua);
	json->Get("dump_dll", dump_dll);
	json->Get("dump_res", dump_res);
    json->Get("dump_res1", dump_res1);
    json->Get("dump_res2", dump_res2);
    json->Get("dump_xxtea", dump_xxtea);
    json->Get("dump_inject", dump_inject);
	delete json;

	G_HookConfig->dump_lua = dump_lua == "1";
	G_HookConfig->dump_dll = dump_dll == "1";
	G_HookConfig->dump_res = dump_res == "1";
    G_HookConfig->dump_res1 = dump_res1 == "1";
    G_HookConfig->dump_res2 = dump_res2 == "1";
    G_HookConfig->dump_xxtea = dump_xxtea == "1";
    G_HookConfig->dump_inject = dump_inject == "1";

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

    int status = G_VM->GetEnv((void **) &env, JNI_VERSION_1_6);
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