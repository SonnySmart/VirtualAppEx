//
// Created by Apple on 2019/4/9.
//

#include "../Utils/Includes.h"

static void * G_Handle = NULL;
static size_t G_bWalkResCount = 0;
static size_t G_bWalkLuaCount = 0;
static size_t G_bWriteXXTEA = 0;
static size_t G_bWriteAES = 0;
static char G_filename[1024] = { 0 };

bool check_res(const char *name) {
    return strstr(name, "/res/");
}

bool check_png(const char *name) {
    return (strstr(name, ".png") || strstr(name, ".jpg"));
}

bool check_src(const char *name) {
    return strstr(name, "/src/");
}

bool check_lua(const char *name) {
    return strstr(name, ".lua");
}

bool check_zip(const char *name) {
    return strstr(name, ".zip");
}

/* file dump start */
//static FileUtils* getInstance();
HOOK_DEF(void *, FileUtils_getInstance) {
    return old_FileUtils_getInstance();
}

//unsigned char* getFileData(const std::string& filename, const char* mode, ssize_t *size)
HOOK_DEF(unsigned char *, getFileData_3x, void *self, const std::string& filename, const char* mode, ssize_t *size) {
    DUALLOGD("[+] [%s] filename[%s]", __FUNCTION__, filename.c_str());
    return old_getFileData_3x(self, filename, mode, size);
}

//unsigned char* getFileData(const char* pszFileName, const char* pszMode, unsigned long * pSize)
HOOK_DEF(unsigned char *, getFileData_2x, void *self, const char* pszFileName, const char* pszMode, unsigned long * pSize) {
    return old_getFileData_2x(self, pszFileName, pszMode, pSize);
}

WALK_FUNC(getFileData) {
    unsigned long size = 0;
    void *buffer = NULL;
    if (old_getFileData_3x && param) buffer = old_getFileData_3x(param, name, "rb", (ssize_t *)&size);
    else if (old_getFileData_2x && param) buffer = old_getFileData_2x(param, name, "rb", &size);

    if (buffer && size > 0)
        dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(name), (const char *)buffer, size);
}

/* file dump end */

/* res dump start */
HOOK_DEF(void, Image, void *self) {
    old_Image(self);
}

//Sprite* Sprite::create(const std::string& filename)
HOOK_DEF(void *, Sprite_create, const std::string& filename) {
    DUALLOGD("+ [%s] filename[%s]", __FUNCTION__, filename.c_str());
    return old_Sprite_create(filename);
}

WALK_FUNC(Image) {
    if (check_lua(name))
        return;

    memset(G_filename, 0, sizeof(G_filename));
    snprintf(G_filename, sizeof(G_filename), "%s", name);

    DUALLOGD("filename[%s]", G_filename);

    if (old_Sprite_create) old_Sprite_create(G_filename);
}

//Image::Format Image::detectFormat(const unsigned char * data, ssize_t dataLen)
HOOK_DEF(int, detectFormat, void *self, const unsigned char * data, ssize_t dataLen) {
    DUALLOGD("[+] [%s] data[%p] len[%d] name[%s]", __FUNCTION__, data, dataLen, G_filename);

    if (strstr(G_filename, TEMP_PATH) && data && dataLen > 0)
    {
#if 1
        dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(G_filename), (const char *)data, dataLen);
#else
        char buffer[128] = { 0 };
        sprintf(buffer, "%p.png", data);
        dump_write(PACK_NAME, ASSET_PATH, buffer, (const char *)data, dataLen);
#endif
    }

    return old_detectFormat(self, data, dataLen);
}

/* res dump end */

/* lua dump start */

//void  (lua_pushstring) (lua_State *L, const char *s);
HOOK_DEF(void, lua_pushstring, void *L, const char *s) {
    old_lua_pushstring(L, s);
}

//void  (lua_settop) (lua_State *L, int idx);
HOOK_DEF(void ,lua_settop, void *L, int idx) {
    old_lua_settop(L, idx);
}

void *G_lua_State = NULL;
//void luaL_openlibs(lua_State *L);
HOOK_DEF(void , luaL_openlibs, void *L) {
    G_lua_State = L;
    DUALLOGD("[+] [%s] G_lua_State[%p]", __FUNCTION__, G_lua_State);
    old_luaL_openlibs(L);
}

//int cocos2dx_lua_loader(lua_State *L)
HOOK_DEF(int, cocos2dx_lua_loader, void *L) {
    return old_cocos2dx_lua_loader(L);
}

//int LuaEngine::executeScriptFile(const char* filename)
HOOK_DEF(int, executeScriptFile, void *self, const char* filename) {
    DUALLOGD("[+] [%s] self[%p] filename[%s]", __FUNCTION__, self, filename);
    return old_executeScriptFile(self, filename);
}

WALK_FUNC(luaLoadBuffer) {
    if (!check_lua(name))
        return;
    if (old_executeScriptFile && param) {
        DUALLOGD("[+] [%s] old_executeScriptFile[%p] name[%s]", __FUNCTION__, param, name);
        old_executeScriptFile(param, name);
    }
    else if (G_lua_State) {
        DUALLOGD("[+] [%s] G_lua_State[%p] name[%s]", __FUNCTION__, G_lua_State, name);
        old_lua_settop(G_lua_State, 0);
        old_lua_pushstring(G_lua_State, name);
        old_cocos2dx_lua_loader(G_lua_State);
    }
}

//static LuaEngine* getInstance(void);
HOOK_DEF(void *, LuaEngine_getInstance) {
    return old_LuaEngine_getInstance();
}

//int (luaL_loadbuffer) (lua_State *L, const char *buff, size_t sz, const char *name);
HOOK_DEF(int, luaL_loadbuffer, void *L, const char *buff, size_t size, const char *name) {
    DUALLOGD("[+] [%s] L[%p] buff[%p] size[%d] name[%s]", __FUNCTION__, L, buff, size, name);

#if 0
    call_stack();
#endif

    if (G_HookConfig->dump_lua) {
        if (strstr(name, TEMP_PATH) && buff && size > 0)
            dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(name), buff, size);
    }

    if (G_HookConfig->dump_inject) {
        void *out_buffer = NULL;
        size_t out_len = 0;
        std::vector<std::string> r {
                "/RedEnvelopeLayer.lua",
                "/GameLayer.lua",
                "/GameViewLayer.lua",
                //"/functions.lua",
                //"/GameFrameEngine.lua"
        };
        if (replace_buffer(INJECT_PATH, name, r, out_buffer, out_len) == 0) {
            DUALLOGD("注入lua[%s]成功", name);
        }
        if (out_buffer && out_len > 0) return old_luaL_loadbuffer(L, (const char *)out_buffer, out_len, name);
        return old_luaL_loadbuffer(L, buff, size, name);
    }
    else {
        return old_luaL_loadbuffer(L, buff, size, name);
    }
}

/* lua dump end */

//unsigned char *xxtea_decrypt(unsigned char *data, xxtea_long data_len, unsigned char *key, xxtea_long key_len, xxtea_long *ret_length);
HOOK_DEF(unsigned char *, xxtea_decrypt, unsigned char *data, unsigned int data_len, unsigned char *key, unsigned int key_len, unsigned int *ret_length) {
    DUALLOGD("[+] [%s] key[%s] key_len[%d]", __FUNCTION__, key, key_len);
    //DUALLOGD("%s", data);
    //if (G_bWriteXXTEA++ == 5)
    {
        const char *mode = G_bWriteXXTEA++ == 0 ? "w" : "a+";
        FILE *fp = NULL;
        char buff[512] = { 0 };
        char keybuff[128] = { 0 };
        memcpy(keybuff, key, key_len);
        snprintf(buff, sizeof(buff), "[+] package[%s] xxtea key[%s] key_len[%d]\n", PACK_NAME, keybuff, key_len);

        do {
            if ((fp = fopen(XXTEA_FILE, mode)) == NULL)
                break;
            if (fwrite(buff, sizeof(char), sizeof(buff), fp) <= 0)
                break;
        } while (0);

        if (fp) fclose(fp);
    }
    return old_xxtea_decrypt(data, data_len, key, key_len, ret_length);
}

//static void DecryptData(uint8_t *contents, uint32_t num_bytes, const aes_key &key);
HOOK_DEF(void, DecryptData, uint8_t *contents, uint32_t num_bytes, const std::array<unsigned char, 32> &key) {
    {
        const char *mode = G_bWriteAES++ == 0 ? "w" : "a+";
        FILE *fp = NULL;
        char buff[512] = { 0 };
        char keybuff[128] = { 0 };
        char hexbuff[256] = { 0 };
        char *pHexPint = hexbuff;
        std::string s(key.begin(), key.end());
        size_t key_len = strlen(s.c_str());
        memcpy(keybuff, s.c_str(), key_len);
        for (int i = 0; i < key_len; ++i) {
            int ret = snprintf(pHexPint, sizeof(hexbuff), "0x%x, ", keybuff[i]);
            if (ret > 0) pHexPint += ret;
        }
        snprintf(buff, sizeof(buff), "[+] package[%s] aes key[%s] hex[%s] key_len[%d]\n", PACK_NAME, keybuff, hexbuff, key_len);

        do {
            if ((fp = fopen(AES_FILE, mode)) == NULL)
                break;
            if (fwrite(buff, sizeof(char), sizeof(buff), fp) <= 0)
                break;
        } while (0);

        if (fp) fclose(fp);
    }
    return old_DecryptData(contents, num_bytes, key);
}

void *G_LuaStack = NULL;
//static LuaStack *create(void);
HOOK_DEF(void *, LuaStack_create) {
    G_LuaStack = old_LuaStack_create();
    return G_LuaStack;
}

//int loadChunksFromZIP(const char *zipFilePath);
HOOK_DEF(int, loadChunksFromZIP, void *self, const char *zipFilePath) {
    DUALLOGD("[+] [%s] zipFilePath[%s]", __FUNCTION__, zipFilePath);
    return old_loadChunksFromZIP(self, zipFilePath);
}

//ZipFile *ZipFile::createWithBuffer(const void* buffer, uLong size)
HOOK_DEF(void *, createWithBuffer, const void* buffer, unsigned long size) {
    DUALLOGD("[+] [%s] buffer[%p] size[%d]", __FUNCTION__, buffer, size);
    const char *name = G_filename;
    if (strstr(name, TEMP_PATH) && buffer && size > 0)
        dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(name), (char *)buffer, size);
    return old_createWithBuffer(buffer, size);
}

WALK_FUNC(loadChunksFromZIP) {
    if (!check_zip(name))
        return;

    memset(G_filename, 0, sizeof(G_filename));
    snprintf(G_filename, sizeof(G_filename), "%s", name);

    if (old_loadChunksFromZIP && G_LuaStack) old_loadChunksFromZIP(G_LuaStack, name);
}

//int get_string_for_print(lua_State * L, std::string* out)
HOOK_DEF(int, get_string_for_print, void *L, std::string* out) {
    int ret = old_get_string_for_print(L, out);
    DUALLOGI("Cocos2dLog[%s]", out->c_str());
    return ret;
}

void start_dump() {
    if (G_HookConfig->dump_lua && G_bWalkLuaCount == 0) {
        DUALLOGD("开始..................................");
        filewalk(TEMP_PATH, WALK_ADDR(luaLoadBuffer), NULL, G_bWalkLuaCount, false);
    }
    if (G_HookConfig->dump_res && G_bWalkResCount == 0) {
        DUALLOGD("开始..................................");
        filewalk(TEMP_PATH, WALK_ADDR(getFileData), old_FileUtils_getInstance(), G_bWalkResCount, false);
    }
    if (G_HookConfig->dump_res1 && G_bWalkResCount == 0) {
        DUALLOGD("开始..................................");
        filewalk(TEMP_PATH, WALK_ADDR(Image), NULL, G_bWalkResCount, false);
    }
    if (G_HookConfig->dump_res2 && G_bWalkResCount == 0) {
        DUALLOGD("开始..................................");
        filewalk(TEMP_PATH, WALK_ADDR(loadChunksFromZIP), NULL, G_bWalkResCount, false);
    }
}

extern void unshell_so_entry(const char *name, void *handle);

//bool AppDelegate::applicationDidFinishLaunching()
HOOK_DEF(bool ,applicationDidFinishLaunching, void *self) {
    bool ret = old_applicationDidFinishLaunching(self);
    //unshell_so_entry(NULL, NULL);
    start_dump();
    return ret;
}

//int Application::run()
HOOK_DEF(int, Application_run, void *self) {
    int ret = old_Application_run(self);
    start_dump();
    return ret;
}

#define SELL
void cocos_entry(const char *name, void *handle)
{
    //toast("cocos 开始注入...");
    G_Handle = handle;
    G_bWalkLuaCount = 0;
    G_bWalkResCount = 0;
    G_bWriteXXTEA = 0;
    G_bWriteAES = 0;

    unsigned long base = 0;
    findLibBase(HOOK_NAME, &base);

#ifdef SELL
    //HOOK启动函数
    //_ZN11AppDelegate29applicationDidFinishLaunchingEv
    MS(handle, "_ZN11AppDelegate29applicationDidFinishLaunchingEv", applicationDidFinishLaunching);
#else
    //HOOK启动函数
    MS(handle, "_ZN7cocos2d11Application3runEv", Application_run);
#endif

    if (G_HookConfig->dump_lua)
    {
        /* lua func */
        MS(handle, "lua_pushstring", lua_pushstring);
        MS(handle, "lua_settop", lua_settop);
        MS(handle, "luaL_openlibs", luaL_openlibs);
        MS(handle, "luaL_loadbuffer", luaL_loadbuffer);
        /* lua func */
        MS(handle, "cocos2dx_lua_loader", cocos2dx_lua_loader);
        //MS(handle, "_ZN7cocos2d9LuaEngine11getInstanceEv", LuaEngine_getInstance);
        //MS(handle, "_ZN7cocos2d9LuaEngine17executeScriptFileEPKc", executeScriptFile);
        //if (old_LuaEngine_getInstance) old_LuaEngine_getInstance();
#if 0
        DUALLOGD("handle 0x[%x] base 0x[%x]", handle, base);
        MS_THUMB(base, 0x388084, luaL_loadbuffer);
#endif
    }

    if (G_HookConfig->dump_res)
    {
        MS(handle, "_ZN7cocos2d9FileUtils11getInstanceEv", FileUtils_getInstance);
        if (!MS(handle, "_ZN7cocos2d16FileUtilsAndroid11getFileDataERKSsPKcPi", getFileData_3x))
            MS(handle, "_ZN7cocos2d9FileUtils11getFileDataERKSsPKcPi", getFileData_3x);
        //start_dump();
    }

    if (G_HookConfig->dump_res1)
    {
        if (!MS(handle, "_ZN7cocos2d5Image12detectFormatEPKhi", detectFormat))
            MS(handle, "_ZN7cocos2d5Image12detectFormatEPKhl", detectFormat);
        if (!MS(handle, "_ZN7cocos2d6Sprite6createERKSs", Sprite_create))
            MS(handle, "_ZN7cocos2d6Sprite6createEv", Sprite_create);
#if 0
        DUALLOGD("handle 0x[%x] base 0x[%x]", handle, base);
        MS_THUMB(base, 0x00548E74, detectFormat);
        MS_THUMB(base, 0x0052F0CC, Sprite_create);
#endif
    }

    if (G_HookConfig->dump_res2)
    {
        MS(handle, "_ZN7cocos2d8LuaStack6createEv", LuaStack_create);
        MS(handle, "_ZN7cocos2d8LuaStack17loadChunksFromZIPEPKc", loadChunksFromZIP);
        MS(handle, "_ZN7cocos2d7ZipFile16createWithBufferEPKvm", createWithBuffer);
    }

    if (G_HookConfig->dump_xxtea)
    {
        MS(handle, "_ZN3ext3AES11DecryptDataEPhjRKSt5arrayIhLj32EE", DecryptData);
        if (!MS(handle, "_Z13xxtea_decryptPhjS_jPj", xxtea_decrypt))
            if (!MS(handle, "_Z8_byds_d_PhjS_jPj", xxtea_decrypt))
                if (!MS(handle, "_Z25xxtea_decrypt_in_cocos2dxPhjS_jPj", xxtea_decrypt))
                    MS(handle, "xxtea_decrypt", xxtea_decrypt);
#if 0
        DUALLOGD("handle 0x[%x] base 0x[%x]", handle, base);
        MS_THUMB(base, 0x321614, xxtea_decrypt);
#endif
    }

    if (G_HookConfig->dump_inject)
    {
        MS(handle, "luaL_loadbuffer", luaL_loadbuffer);
    }

    DUALLOGW("[+] [%s] cocos 注入成功 .", __FUNCTION__);

    //toast("cocos 注入完成...");
}