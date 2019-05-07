//
// Created by Apple on 2019/4/24.
//

#include "../Utils/Includes.h"

#include <unistd.h>
#include <android/log.h>
#include <sys/system_properties.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <string>
#include <dlfcn.h>

int isArt();
void getProcessName(int pid, char *name, int len);
void dumpFileName(char *name, int len, const char *pname, int dexlen);

// 保存当前apk进程的进程名字
static char pname[256] = { 0 };

// 判断当前所处环境是否是Android art虚拟机模式
int isArt() {

    char version[10];

    // 获取ro.build.version.sdk的属性值
    __system_property_get("ro.build.version.sdk", version);
    // 打印当前Android系统的api版本信息
    __android_log_print(ANDROID_LOG_INFO, TAG, "api level %s", version);

    // 将api版本转换成int型版本号
    int sdk = atoi(version);
    // 判断api版本是否是大于21（要求Android系统的版本为 Android 5.0以上 才可以）
    if (sdk >= 21) {

        // art虚拟机模式
        return 1;
    }

    return 0;
}

// 读取/proc/self/cmdline文件的数据，获取当前apk进程的进程名字
void getProcessName(int pid, char *name, int len) {
    int fp;
    char filename[32] = { 0 };
    if (pid > 0)
        sprintf(filename, "/proc/%d/cmdline", pid);
    else
        sprintf(filename, "/proc/%s/cmdline", "self");

    fp = open(filename, O_RDONLY);
    memset(name, 0, len);
    read(fp, name, len);
    close(fp);
}

// 格式字符串构建dump的dex文件的路径字符串
void dumpFileName(char *name, int len, const char *pname, int dexlen) {

    time_t now;
    struct tm *timenow;
    time(&now);
    // 获取当前时间（值得借鉴和学习）
    timenow = localtime(&now);

    memset(name, 0, len);
    // 格式化字符串得到当前dump的dex文件路径字符串
    sprintf(name, "/data/data/%s/dump_size_%u_time_%d_%d_%d_%d_%d_%d.dex", pname, dexlen,
            timenow->tm_year + 1900,
            timenow->tm_mon + 1,
            timenow->tm_mday,
            timenow->tm_hour,
            timenow->tm_min,
            timenow->tm_sec);
}

void writeToFile(const char *pname, u_int8_t *data, size_t length) {

    char dname[1024];

    // pname为当前进程的名称
    // 格式字符串构建dump的dex文件的路径字符串dname
    dumpFileName(dname, sizeof(dname), pname, length);
    __android_log_print(ANDROID_LOG_ERROR, TAG, "dump dex file name is : %s", dname);

    __android_log_print(ANDROID_LOG_ERROR, TAG, "start dump");
    // 根据dname创建新文件用于保存内存dump的dex文件
    int dex = open(dname, O_CREAT | O_WRONLY, 0644);
    if (dex < 0) {

        __android_log_print(ANDROID_LOG_ERROR, TAG, "open or create file error");
        return;
    }

    // 将内存dex文件的数据写入到新的dname文件中
    int ret = write(dex, data, length);
    if (ret < 0) {

        __android_log_print(ANDROID_LOG_ERROR, TAG, "write file error");
    } else {

        __android_log_print(ANDROID_LOG_ERROR, TAG, "dump dex file success `%s`", dname);
    }

    // 关闭文件
    close(dex);
}

// 保存openmemory函数旧的地址
void *(*old_openmemory)(const uint8_t *base, size_t size, const std::string &location,
                                uint32_t location_checksum, void *mem_map,
                                const void *oat_dex_file, std::string *error_msg) = NULL;

void *new_openmemory(const uint8_t *base, size_t size, const std::string &location,
                             uint32_t location_checksum, void *mem_map,
                             const void *oat_dex_file, std::string *error_msg) {

    __android_log_print(ANDROID_LOG_ERROR, TAG, "art::DexFile::OpenMemory is called");

    writeToFile(pname, (uint8_t *) base, size);

    // 调用原art::DexFile::OpenMemory函数
    return (*old_openmemory)(base, size, location, location_checksum, mem_map, oat_dex_file,
                             error_msg);
}

void unshell_dex_entry()
{
    DUALLOGD("[+] [%s] call .", __FUNCTION__);

    // 获取当前apk进程的进程名字
    getProcessName(getpid(), pname, sizeof(pname));

    void *handle = dlopen("libart.so", RTLD_GLOBAL | RTLD_LAZY);
    if (handle == NULL)
    {
        DUALLOGE("[-] [%s] dlopen[%s]", __FUNCTION__, dlerror());
        return;
    }

    void *symbol = dlsym(handle, "_ZN3art7DexFile10OpenMemoryEPKhjRKNSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEjPNS_6MemMapEPKNS_10OatDexFileEPS9_");
    if (symbol == NULL)
    {
        DUALLOGE("[-] [%s] symbol[%s]", __FUNCTION__, dlerror());
        return;
    }

    DUALLOGD("[+] [%s] OpenMemory find .", __FUNCTION__);

    //MSHookFunction(symbol, (void *)&new_openmemory, (void **)&old_openmemory);
}