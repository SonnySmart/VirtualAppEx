//
// Created by Apple on 2019/4/9.
//

#include "../Utils/Includes.h"
#include "CocosDef.h"

extern unsigned char *com_yoyo_dygame_GameApplication_decrypt(unsigned char *data, unsigned long len, unsigned long &out_len);

struct xxtea_param {
    std::string key;
    size_t key_len;
};

struct luaL_param {
    void *L;
    void *self;
};

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

void *decrypt_ptr = nullptr;

void *OLD_FUNC_PTR(decrypt)(void *self, void *data);
void *NEW_FUNC(decrypt)(void *self, void *data) {
    decrypt_ptr = self;
    return OLD_FUNC(decrypt)(self, data);
}

Data OLD_FUNC_PTR(getDataFromFile)(void *self, const std::string& filename);
WALK_FUNC(getDataFromFile)
{
    if (!check_res(name))
        return;

    Data data = OLD_FUNC(getDataFromFile)(param, name);
    if (data.getBytes() && data.getSize() > 0) {
        //dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(name), (const char *)data.getBytes(), data.getSize());
        //DUALLOGD("[+] [%s] name[%s]", __FUNCTION__, name);
        //OLD_FUNC(decrypt)(decrypt_ptr, &data);
    }
}
Data NEW_FUNC(getDataFromFile)(void *self, const std::string& filename)
{
    filewalk(TEMP_PATH, WALK_ADDR(getDataFromFile), self, G_walkCount);
    return OLD_FUNC(getDataFromFile)(self, filename);
}

std::string OLD_FUNC_PTR(getStringFromFile)(void *self, const std::string& filename);
WALK_FUNC(getStringFromFile)
{
    if (!check_lua(name))
        return;

    std::string data = OLD_FUNC(getStringFromFile)(param, name);
    //DUALLOGD("data [%s]", data.c_str());
    if (data.size() > 0)
        dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(name), (const char *)data.c_str(), data.size());
}
std::string NEW_FUNC(getStringFromFile)(void *self, const std::string& filename)
{
    filewalk(TEMP_PATH, WALK_ADDR(getStringFromFile), self, G_walkCount);

    return OLD_FUNC(getStringFromFile)(self, filename);
}

Status OLD_FUNC_PTR(getContents)(void *self, const std::string& filename, ResizableBuffer* buffer);
WALK_FUNC(getContents)
{
    if (!check_res(name))
        return;

    ResizableBufferEx data;
    OLD_FUNC(getContents)(param, name, &data);

    if (data.buffer() && data.getSize() > 0)
        dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(name), (const char *)data.buffer(), data.getSize());
}
Status NEW_FUNC(getContents)(void *self, const std::string& filename, ResizableBuffer* buffer)
{
    filewalk(TEMP_PATH, WALK_ADDR(getContents), self, G_walkCount);
    DUALLOGD("[-] self[%p] filename[%s]", self, filename.c_str());
    return OLD_FUNC(getContents)(self, filename, buffer);
}

unsigned char* OLD_FUNC_PTR(getFileData_3x)(void *self, const std::string& filename, const char* mode, ssize_t *size);
WALK_FUNC(getFileData_3x)
{
    if (!check_res(name))
        return;

    ssize_t size = 0;
    unsigned char *ret = OLD_FUNC(getFileData_3x)(param, name, "rb", &size);

    if (ret && size > 0)
        dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(name), (const char *)ret, size);
}
unsigned char* NEW_FUNC(getFileData_3x)(void *self, const std::string& filename, const char* mode, ssize_t *size)
{
    filewalk(TEMP_PATH, WALK_ADDR(getFileData_3x), self, G_walkCount);
    DUALLOGD("[-] self[%p] filename[%s] size[%d]", self, filename.c_str(), *size);
    return OLD_FUNC(getFileData_3x)(self, filename, mode, size);
}

unsigned char* OLD_FUNC_PTR(getFileData_2x)(void *self, const char* pszFileName, const char* pszMode, unsigned long * pSize);
WALK_FUNC(getFileData_2x)
{
    if (!check_res(name))
        return;

    ssize_t size = 0;
    unsigned char *ret = OLD_FUNC(getFileData_2x)(param, name, "rb", (unsigned long *)&size);

    if (ret && size > 0)
        dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(name), (const char *)ret, size);
}
unsigned char* NEW_FUNC(getFileData_2x)(void *self, const char* pszFileName, const char* pszMode, unsigned long * pSize)
{
    filewalk(TEMP_PATH, WALK_ADDR(getFileData_2x), self, G_walkCount);
    DUALLOGD("[-] self[%p] filename[%s] size[%d]", self, pszFileName, *pSize);
    return OLD_FUNC(getFileData_2x)(self, pszFileName, pszMode, pSize);
}

int OLD_FUNC_PTR(detectFormat)(void *self, const unsigned char * data, ssize_t dataLen);
int NEW_FUNC(detectFormat)(void *self, const unsigned char * data, ssize_t dataLen)
{
    if (G_walkCount == 1)
    {
        DUALLOGD("[+] [%s] data[%p] len[%d] name[%s]", __FUNCTION__, data, dataLen, tmp_filename.c_str());
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
    return OLD_FUNC(detectFormat)(self, data, dataLen);
}

bool OLD_FUNC_PTR(initWithImageData)(void *self, const unsigned char * data, ssize_t dataLen);
WALK_FUNC(initWithImageData)
{
    if (check_res(name))
    {
        DUALLOGD("[+] [%s] name[%s] buff[%p] len[%d]", __FUNCTION__, name, buff, len);

        if (check_png(name))
        {
            tmp_filename = name;
            //DUALLOGI("buff[%p], len[%d] name[%s]", buff, len, name);
            OLD_FUNC(initWithImageData)(param, (const unsigned char *)buff, len);
        }
        else
        {
            dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(name), (const char *)buff, len);
        }
    }
}
bool NEW_FUNC(initWithImageData)(void*self, const unsigned char * data, ssize_t dataLen)
{
#if 0
    filewalk(TEMP_PATH, WALK_ADDR(initWithImageData), self, G_walkCount);
    //DUALLOGI("[+] %s data[%p] len[%d]", __FUNCTION__, data, dataLen);
#else
    char buffer[128] = { 0 };
    sprintf(buffer, "%p.png", data);
    dump_write(PACK_NAME, ASSET_PATH, buffer, (const char *)data, dataLen);
#endif
    return OLD_FUNC(initWithImageData)(self, data, dataLen);
}

//bool initWithRawData(const unsigned char * data, ssize_t dataLen, int width, int height, int bitsPerComponent, bool preMulti = false);
bool NEW_FUNC(initWithRawData)(void *self, const unsigned char * data, ssize_t dataLen, int width, int height, int bitsPerComponent, bool preMulti)
{

}

bool OLD_FUNC_PTR(initWithImageFile)(void *self, const std::string& path);
WALK_FUNC(initWithImageFile)
{
    if (check_res(name))
    {
        if (check_png(name))
        {
            tmp_filename = name;
            //DUALLOGI("buff[%p], len[%d] name[%s]", buff, len, name);
            OLD_FUNC(initWithImageFile)(param, tmp_filename);
        }
        else
        {
            dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(name), (const char *)buff, len);
        }
    }
}
bool NEW_FUNC(initWithImageFile)(void *self, const std::string& path)
{
    filewalk(TEMP_PATH, WALK_ADDR(initWithImageFile), self, G_walkCount);
    return OLD_FUNC(initWithImageFile)(self, path);
}
/* res dump */

int OLD_FUNC_PTR(luaL_loadbuffer)(void *L, const char *buff, size_t size, const char *name);
int NEW_FUNC(luaL_loadbuffer)(void *L, const char *buff, size_t size, const char *name)
{
#if 0
    if (!G_injectLua++)
    {
        const char *inject_name = "inject.lua";
        std::string inject = INJECT_PATH;
        inject.append("/").append(inject_name);

        char *buffer = NULL;
        size_t len = 0;
        read_buffer(inject.c_str(), buffer, len);

        if (buffer && len > 0)
        {
            old_luaL_loadbuffer(L, buffer, len, inject_name);
            DUALLOGI("[+] [%s] inject[%s] buffer[%p] len[%d] 注入成功 .", __FUNCTION__, inject_name, buffer, len);
        }
    }
#endif
#if 0
//local f = loadstring(convertToLuaCode(code))() return f
    if (strstr(name, "/qpby/"))
    {
        //dump_write(PACK_NAME, ASSET_PATH, name, (const char *)buff, size);

        char newString[1024] = { 0 };
        std::string filename = ASSET_PATH;
        filename.append("/").append(PACK_NAME).append("/").append(name);

        // 查找文件后缀
        std::string::size_type npos = filename.find_last_of('.');
        if (npos == std::string::npos)
            return -1;

        // 获取文件后缀
        std::string suffixStr = filename.substr(npos + 1);
        // luac替换为lua
        if (suffixStr == "luac")
            filename = filename.substr(0, npos).append(".lua");

        sprintf(newString, "local decode = convertToLuaCode(code) local file = io.open(\"%s\", \"w\") file:write(decode) file:close() local f = loadstring(decode)() return f", filename.c_str());
        std::string luaString = buff;
        npos = luaString.find_last_of('\"');
        //std::string::size_type find = luaString.find_last_of("local f = loadstring(convertToLuaCode(code))() return f");
        if ((strstr(name, "/models/") || strstr(name, "/views/")) && npos != std::string::npos)
        {
            luaString = luaString.substr(0, npos + 2);
            luaString.append(newString);

            DUALLOGI("[+] [%s] buff[%p] size[%d] name[%s]", __FUNCTION__, buff, size, name);

            return old_luaL_loadbuffer(L, luaString.c_str(), luaString.length(), name);
        }
    }
#endif

    //DUALLOGD("[+] [%s] buff[%p] size[%d] name[%s]", __FUNCTION__, buff, size, name);
    if (strstr(name, TEMP_PATH) && buff && size > 0)
        dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(name), buff, size);
    return old_luaL_loadbuffer(L, buff, size, name);
}

int OLD_FUNC_PTR(luaLoadBuffer)(void *self, void *L, const char *chunk, int chunkSize, const char *chunkName);
WALK_FUNC(luaLoadBuffer)
{
    if (!check_lua(name))
        return;

    DUALLOGD("[+] [%s] name[%s] buff[%p] len[%d] self[%p] L[%p]", __FUNCTION__, name, buff, len, ((luaL_param *)param)->self, ((luaL_param *)param)->L);

    OLD_FUNC(luaLoadBuffer)(((luaL_param *)param)->self, ((luaL_param *)param)->L, buff, len, name);
}
int NEW_FUNC(luaLoadBuffer)(void *self, void *L, const char *chunk, int chunkSize, const char *chunkName)
{
    luaL_param param;
    param.L = L;
    param.self = self;
    filewalk(TEMP_PATH, WALK_ADDR(luaLoadBuffer), &param, G_walkCount);

    //DUALLOGD("[+] [%s] buff[%p] size[%d] name[%s]", __FUNCTION__, chunk, chunkSize, chunkName);
    return OLD_FUNC(luaLoadBuffer)(self, L, chunk, chunkSize, chunkName);
}

unsigned char *OLD_FUNC_PTR(xxtea_decrypt)(unsigned char *data, unsigned int data_len, unsigned char *key, unsigned int key_len, unsigned int *ret_length);
WALK_FUNC(xxtea_decrypt)
{
    if (!check_lua(name))
        return;

    DUALLOGD("[+] [%s] name[%s] buff[%p] len[%d]", __FUNCTION__, name, buff, len);

    size_t sign_len = 8;
    size_t ret_len = 0;
    unsigned char *ret = OLD_FUNC(xxtea_decrypt)((unsigned char *)buff + sign_len, len - sign_len, (unsigned char *)((xxtea_param *)param)->key.c_str(), ((xxtea_param *)param)->key_len, &ret_len);
    if (ret && ret_len > 0)
        dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(name), (const char *)ret, ret_len);

    if (ret) free(ret);
}
unsigned char *NEW_FUNC(xxtea_decrypt)(unsigned char *data, unsigned int data_len, unsigned char *key, unsigned int key_len, unsigned int *ret_length)
{
    xxtea_param param;
    param.key = (char *)key;
    param.key_len = key_len;
    //filewalk(TEMP_PATH, WALK_ADDR(xxtea_decrypt), &param, G_walkCount);

    DUALLOGD("[+] [%s] key[%s] key_len[%d]", __FUNCTION__, key, key_len);
    return OLD_FUNC(xxtea_decrypt)(data, data_len, key, key_len, ret_length);
}

void cocos_entry(const char *name, void *handle)
{
    G_walkCount = 0;

    if (G_HookConfig->dump_lua)
    {
        MS(handle, "luaL_loadbuffer", luaL_loadbuffer);
        if (!MS(handle, "_ZN7cocos2d8LuaStack13luaLoadBufferEP9lua_StatePKciS4_", luaLoadBuffer))
            MS(handle, "_ZN7cocos2d10CCLuaStack13luaLoadBufferEP9lua_StatePKciS4_", luaLoadBuffer);
    }

    if (G_HookConfig->dump_res)
    {
        if (!MS(handle, "_ZN7cocos2d16FileUtilsAndroid11getFileDataERKSsPKcPi", getFileData_3x))
            MS(handle, "_ZN7cocos2d16FileUtilsAndroid11getContentsERKSsPNS_15ResizableBufferE", getContents);
        MS(handle, "_ZN7cocos2d18CCFileUtilsAndroid11getFileDataEPKcS2_Pm", getFileData_2x);
    }

    if (G_HookConfig->dump_res1)
    {
        //MS(handle, "_ZN7cocos2d5Image17initWithImageDataEPKhi", initWithImageData);
        MS(handle, "_ZN7cocos2d5Image17initWithImageFileERKSs", initWithImageFile);
        MS(handle, "_ZN7cocos2d5Image12detectFormatEPKhi", detectFormat);
    }

    if (G_HookConfig->dump_res2)
    {
        //猫娱乐资源
        //MS(handle, "_ZN16FileUtilsEncrypt15getDataFromFileERKSs", getDataFromFile);
        //MS(handle, "_ZN16FileUtilsEncrypt17getStringFromFileERKSs", getStringFromFile);
        if (!MS(handle, "_ZN7cocos2d16FileUtilsAndroid15getDataFromFileERKSs", getDataFromFile))
            MS(handle, "_ZN7cocos2d9FileUtils15getDataFromFileERKSs", getDataFromFile);
    }

    if (G_HookConfig->dump_xxtea)
    {
        if (!MS(handle, "_Z13xxtea_decryptPhjS_jPj", xxtea_decrypt))
            if (!MS(handle, "_Z8_byds_d_PhjS_jPj", xxtea_decrypt))
                MS(handle, "_Z25xxtea_decrypt_in_cocos2dxPhjS_jPj", xxtea_decrypt);
    }

    DUALLOGW("[+] [%s] end", __FUNCTION__);

    //toast("cocos 注入成功");
}