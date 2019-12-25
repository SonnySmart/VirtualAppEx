//
// Created by Apple on 2019/4/9.
//

#include "../Utils/Includes.h"
#include "CocosDef.h"

size_t G_walkCount = 0;
size_t G_injectLua = 0;

std::string tmp_filename = "";

extern bool check_lua(const char *name);
extern bool check_res(const char *name);

bool check_res(const char *name)
{
    return strstr(name, "/res/");
}

bool check_png(const char *name)
{
    return (strstr(name, ".png") || strstr(name, ".jpg"));
}

bool check_src(const char *name)
{
    return strstr(name, "/src/");
}

bool check_lua(const char *name)
{
    return strstr(name, ".lua");
}

/* file dump start */
//static FileUtils* getInstance();
HOOK_DEF(void *, FileUtils_getInstance) {
    return old_FileUtils_getInstance();
}

//unsigned char* getFileData(const std::string& filename, const char* mode, ssize_t *size)
HOOK_DEF(unsigned char *, getFileData_3x, void *self, const std::string& filename, const char* mode, ssize_t *size) {
    return old_getFileData_3x(self, filename, mode, size);
}

//unsigned char* getFileData(const char* pszFileName, const char* pszMode, unsigned long * pSize)
HOOK_DEF(unsigned char *, getFileData_2x, void *self, const char* pszFileName, const char* pszMode, unsigned long * pSize) {
    return old_getFileData_2x(self, pszFileName, pszMode, pSize);
}

WALK_FUNC(getFileData) {
    unsigned long size = 0;
    void *buffer = NULL;
    if (old_getFileData_3x) buffer = old_getFileData_3x(param, name, "rb", (ssize_t *)&size);
    else if (old_getFileData_2x) buffer = old_getFileData_2x(param, name, "rb", &size);

    if (buffer && size > 0)
        dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(name), (const char *)buffer, size);
}
/* file dump end */

/* res dump start */
HOOK_DEF(void, Image, void *self) {
    old_Image(self);
}

//bool Image::initWithImageFile(const std::string& path)
HOOK_DEF(bool, initWithImageFile, void *self, const std::string& path) {
    //DUALLOGD("[+] [%s] self[%p] path[%s]", __FUNCTION__, self, path.c_str());
    tmp_filename = path;
    return old_initWithImageFile(self, path);
}

//Image::Format Image::detectFormat(const unsigned char * data, ssize_t dataLen)
HOOK_DEF(int, detectFormat, void *self, const unsigned char * data, ssize_t dataLen) {
    if (G_walkCount == 1)
    {
        //DUALLOGD("[+] [%s] data[%p] len[%d] name[%s]", __FUNCTION__, data, dataLen, tmp_filename.c_str());
        if (data && dataLen > 0)
        {
#if 1
            dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(tmp_filename.c_str()), (const char *)data, dataLen);
#else
            char buffer[128] = { 0 };
            sprintf(buffer, "%p.png", data);
            dump_write(PACK_NAME, ASSET_PATH, buffer, (const char *)data, dataLen);
#endif
        }
    }
    return old_detectFormat(self, data, dataLen);
}

WALK_FUNC(Image) {
    void *image = operator new (0xC0u);
    new_Image(image);
    new_initWithImageFile(image, name);
    delete(image);
}

/* res dump end */

/* lua dump start */
HOOK_DEF(int, luaL_loadbuffer, void *L, const char *buff, size_t size, const char *name) {
    //DUALLOGD("[+] [%s] buff[%p] size[%d] name[%s]", __FUNCTION__, buff, size, name);
    if (strstr(name, TEMP_PATH) && buff && size > 0)
        dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(name), buff, size);
    return old_luaL_loadbuffer(L, buff, size, name);
}

//int LuaEngine::executeScriptFile(const char* filename)
HOOK_DEF(int, executeScriptFile, void *self, const char* filename) {
    DUALLOGD("[+] [%s] self[%p] filename[%s]", __FUNCTION__, self, filename);
    return old_executeScriptFile(self, filename);
}

WALK_FUNC(luaLoadBuffer) {
    if (!check_lua(name))
        return;
    old_executeScriptFile(param, name);
}

//static LuaEngine* getInstance(void);
HOOK_DEF(void *, LuaEngine_getInstance) {
    return old_LuaEngine_getInstance();
}

/* lua dump end */

//unsigned char *xxtea_decrypt(unsigned char *data, xxtea_long data_len, unsigned char *key, xxtea_long key_len, xxtea_long *ret_length);
HOOK_DEF(unsigned char *, xxtea_decrypt, unsigned char *data, unsigned int data_len, unsigned char *key, unsigned int key_len, unsigned int *ret_length) {
    DUALLOGD("[+] [%s] key[%s] key_len[%d]", __FUNCTION__, key, key_len);
    return old_xxtea_decrypt(data, data_len, key, key_len, ret_length);
}

HOOK_DEF(int, lua_gettop, void *L) {
    DUALLOGE("[+] fn[%s] L[%p]", __FUNCTION__, L);
    if (L == NULL)
        return 0;
    int ret = old_lua_gettop(L);
    DUALLOGE("[+] fn[%s] ret[%d]", __FUNCTION__, ret);
    return ret;
}

void cocos_entry(const char *name, void *handle)
{
    G_walkCount = 0;

    if (G_HookConfig->dump_lua)
    {
        MS(handle, "luaL_loadbuffer", luaL_loadbuffer);
        if (MS(handle, "_ZN7cocos2d9LuaEngine11getInstanceEv", LuaEngine_getInstance))
            if (MS(handle, "_ZN7cocos2d9LuaEngine17executeScriptFileEPKc", executeScriptFile))
            {
                DUALLOGD("dump_lua call .");
                filewalk(TEMP_PATH, WALK_ADDR(luaLoadBuffer), new_LuaEngine_getInstance(), G_walkCount, false);
            }
    }

    if (G_HookConfig->dump_res)
    {
        if (MS(handle, "_ZN7cocos2d9FileUtils11getInstanceEv", FileUtils_getInstance))
            if (MS(handle, "_ZN7cocos2d9FileUtils11getFileDataERKSsPKcPi", getFileData_3x))
            {
                DUALLOGD("dump_file call .");
                filewalk(TEMP_PATH, WALK_ADDR(getFileData), new_FileUtils_getInstance(), G_walkCount, false);
            }
    }

    if (G_HookConfig->dump_res1)
    {
        if (MS(handle, "_ZN7cocos2d5Image17initWithImageFileERKSs", initWithImageFile))
            if (MS(handle, "_ZN7cocos2d5Image12detectFormatEPKhi", detectFormat))
                if (MS(handle, "_ZN7cocos2d5ImageC2Ev", Image))
                {
                    DUALLOGD("dump_res1 call .");
                    filewalk(TEMP_PATH, WALK_ADDR(Image), nullptr, G_walkCount, false);
                }
    }

    if (G_HookConfig->dump_res2)
    {
        DUALLOGE("dump_res2 没有实现");
    }

    if (G_HookConfig->dump_xxtea)
    {
        //if (!MS(handle, "_ZN7cocos2d5extra6Crypto12decryptXXTEAEPhiS2_iPi", xxtea_decrypt))
            if (!MS(handle, "_Z13xxtea_decryptPhjS_jPj", xxtea_decrypt))
                if (!MS(handle, "_Z8_byds_d_PhjS_jPj", xxtea_decrypt))
                    MS(handle, "_Z25xxtea_decrypt_in_cocos2dxPhjS_jPj", xxtea_decrypt);
    }

    //MS(handle, "lua_gettop", lua_gettop);

    DUALLOGW("[+] [%s] cocos 注入成功 .", __FUNCTION__);

    //toast("cocos 注入成功");
}