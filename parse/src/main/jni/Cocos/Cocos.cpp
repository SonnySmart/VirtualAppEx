//
// Created by Apple on 2019/4/9.
//

#include "../Utils/Includes.h"

static size_t G_bWalkResCount = 0;
static size_t G_bWalkLuaCount = 0;
static char G_filename[1024] = { 0 };
//std::string G_filename;

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

//bool Image::initWithImageFile(const std::string& path)
HOOK_DEF(bool, initWithImageFile, void *self, const std::string& path) {
    //DUALLOGD("[+] [%s] self[%p] path[%s]", __FUNCTION__, self, path.c_str());
    return old_initWithImageFile(self, path);
}

//Sprite* Sprite::create(const std::string& filename)
HOOK_DEF(void *, Sprite_create, const std::string& filename) {
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
//lua_State *(luaL_newstate) (void);
HOOK_DEF(void *, luaL_newstate) {
    G_lua_State = old_luaL_newstate();
    DUALLOGD("[+] [%s] G_lua_State[%p]", __FUNCTION__, G_lua_State);
    return G_lua_State;
}

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
    DUALLOGD("[+] [%s] L[%p] buff[%s] size[%d] name[%s]", __FUNCTION__, L, buff, size, name);

    //if (strstr(name, "main.lua"))
#if 0
    call_stack();
#endif

    if (strstr(name, TEMP_PATH) && buff && size > 0)
        dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(name), buff, size);
    return old_luaL_loadbuffer(L, buff, size, name);
}

/* lua dump end */

//unsigned char *xxtea_decrypt(unsigned char *data, xxtea_long data_len, unsigned char *key, xxtea_long key_len, xxtea_long *ret_length);
HOOK_DEF(unsigned char *, xxtea_decrypt, unsigned char *data, unsigned int data_len, unsigned char *key, unsigned int key_len, unsigned int *ret_length) {
    DUALLOGD("[+] [%s] key[%s] key_len[%d]", __FUNCTION__, key, key_len);
    return old_xxtea_decrypt(data, data_len, key, key_len, ret_length);
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
}

//bool AppDelegate::applicationDidFinishLaunching()
HOOK_DEF(bool ,applicationDidFinishLaunching) {
    bool ret = old_applicationDidFinishLaunching();
    start_dump();
    return ret;
}

void cocos_entry(const char *name, void *handle)
{
    //toast("cocos 开始注入...");

    G_bWalkLuaCount = 0;
    G_bWalkResCount = 0;

    //HOOK启动函数
    MS(handle, "_ZN11AppDelegate29applicationDidFinishLaunchingEv", applicationDidFinishLaunching);

    if (G_HookConfig->dump_lua)
    {
        /* lua func */
        MS(handle, "lua_pushstring", lua_pushstring);
        MS(handle, "lua_settop", lua_settop);
        //MS(handle, "luaL_newstate", luaL_newstate);
        MS(handle, "luaL_openlibs", luaL_openlibs);
        MS(handle, "luaL_loadbuffer", luaL_loadbuffer);
        /* lua func */
        MS(handle, "cocos2dx_lua_loader", cocos2dx_lua_loader);
        //MS(handle, "_ZN7cocos2d9LuaEngine11getInstanceEv", LuaEngine_getInstance);
        //MS(handle, "_ZN7cocos2d9LuaEngine17executeScriptFileEPKc", executeScriptFile);
        //if (old_LuaEngine_getInstance) old_LuaEngine_getInstance();
    }

    if (G_HookConfig->dump_res)
    {
        MS(handle, "_ZN7cocos2d9FileUtils11getInstanceEv", FileUtils_getInstance);
        MS(handle, "_ZN7cocos2d9FileUtils11getFileDataERKSsPKcPi", getFileData_3x);
    }

    if (G_HookConfig->dump_res1)
    {
        //MS(handle, "_ZN7cocos2d5Image17initWithImageFileERKSs", initWithImageFile);
        //MS(handle, "_ZN7cocos2d5ImageC2Ev", Image);
        MS(handle, "_ZN7cocos2d5Image12detectFormatEPKhi", detectFormat);
        MS(handle, "_ZN7cocos2d6Sprite6createERKSs", Sprite_create);
    }

    if (G_HookConfig->dump_res2)
    {
        DUALLOGE("dump_res2 没有实现");
    }

    if (G_HookConfig->dump_xxtea)
    {
        MS(handle, "_Z13xxtea_decryptPhjS_jPj", xxtea_decrypt);
        MS(handle, "_Z8_byds_d_PhjS_jPj", xxtea_decrypt);
        MS(handle, "_Z25xxtea_decrypt_in_cocos2dxPhjS_jPj", xxtea_decrypt);
    }

    DUALLOGW("[+] [%s] cocos 注入成功 .", __FUNCTION__);

    //toast("cocos 注入完成...");
}