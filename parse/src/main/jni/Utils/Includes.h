//
// Created by Apple on 2019/4/9.
//

#ifndef __UTILS_INCLUDES__
#define __UTILS_INCLUDES__

#define WHALE 0

#include "Misc/Misc.h"
#include "CJsonObject/CJsonObject.hpp"
#include "Substrate/CydiaSubstrate.h"
#include "dlfcn/include/dlfcn_compat.h"
#include "Foundation/SymbolFinder.h"
#include "Foundation/SandboxFs.h"
#include "Foundation/Path.h"
#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <exception>

#define TAG "myhook"
#define DUALLOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__);printf(__VA_ARGS__)
#define DUALLOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__);printf(__VA_ARGS__)
#define DUALLOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__);printf(__VA_ARGS__)
#define DUALLOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__);printf(__VA_ARGS__)

#define LOG_TAG "myhook"
# define ALOGV(...) DUALLOGD(__VA_ARGS__)
# define ALOGD(...) DUALLOGD(__VA_ARGS__)
# define ALOGI(...) DUALLOGI(__VA_ARGS__)
# define ALOGW(...) DUALLOGW(__VA_ARGS__)
# define ALOGE(...) DUALLOGE(__VA_ARGS__)
# define ALOGF(...) DUALLOGD(__VA_ARGS__)

#define TEMP_PATH "/sdcard/myhook/tmp"
#define ASSET_PATH "/sdcard/myhook/Cocos2dAsset"
#define INJECT_PATH "/sdcard/myhook/inject"
#define ASSET_NAME(name) assets_name(name, TEMP_PATH)
#define PACK_NAME G_HookConfig->pack_name.c_str()
#define HOOK_NAME G_HookConfig->hook_name.c_str()

#define OLD_FUNC_PTR(fn) (* old_##fn)
#define OLD_FUNC(fn) old_##fn
#define NEW_FUNC(fn) new_##fn
#define WALK_FUNC(fn) void fn##_buffer(char *name, char *buff, size_t len, void *param)
#define WALK_ADDR(fn) &fn##_buffer
#define MS(handle, symbol, fn) inline_hook(handle, symbol, reinterpret_cast<void *>(NEW_FUNC(fn)), reinterpret_cast<void **>(&OLD_FUNC(fn)))
#define HOOK_DEF(ret, func, ...) \
  ret (*old_##func)(__VA_ARGS__); \
  ret new_##func(__VA_ARGS__)

typedef struct hook_config {
    std::string pack_name;
    std::string hook_name;
    bool dump_lua;
    bool dump_dll;
    bool dump_res;
    bool dump_res1;
    bool dump_res2;
    bool dump_xxtea;
} *ptr_hook_config;

typedef void (*ptr_WInlineHookFunction)(void *address, void *replace, void **backup);

extern ptr_hook_config G_HookConfig;
extern ptr_WInlineHookFunction G_WInlineHookFunction;
extern void toast(const char *msg);

#endif //__UTILS_INCLUDES__
