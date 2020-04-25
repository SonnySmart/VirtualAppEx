//
// Created by Apple on 2019/4/9.
//

#include "../Utils/Includes.h"
#include <GLES2/gl2.h>

static void * G_Handle = NULL;
static size_t G_bWalkResCount = 0;
static size_t G_bWalkLuaCount = 0;
static size_t G_bDumpLua = 0;
static size_t G_bDumpRes = 0;
static size_t G_bWriteXXTEA = 0;
static char G_filename[1024] = { 0 };
static std::vector<std::string> G_injectFiles;

void start_dump();

bool check_res(const char *name) {
    return strstr(name, "/res/");
}

bool check_png(const char *name) {
    return (strstr(name, ".png") || strstr(name, ".jpg") || strstr(name, ".jpeg") || strstr(name, ".pkm"));
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

HOOK_DEF(void *, Sprite_create_, const char *filename) {
    return old_Sprite_create_(filename);
}

//CCTextureCache * CCTextureCache::sharedTextureCache()
HOOK_DEF(void *, sharedTextureCache) {
    return old_sharedTextureCache();
}

//CCTexture2D * CCTextureCache::addImage(const char * path)
HOOK_DEF(void *, addImage, void *self, const char *path) {
    DUALLOGD("[+] [%s] data[%p] len[%d] name[%s]", __FUNCTION__, self, 0, path);
    if (G_bWalkResCount == 0) start_dump();
    return old_addImage(self, path);
}

HOOK_DEF(bool, initWithImageData, void *self, void * pData,int nDataLen,int eFmt/* = eSrcFmtPng*/,int nWidth/* = 0*/,int nHeight/* = 0*/,int nBitsPerComponent/* = 8*/) {
    DUALLOGD("[+] [%s] data[%p] len[%d] name[%s]", __FUNCTION__, pData, nDataLen, G_filename);
    if (strstr(G_filename, TEMP_PATH) && !check_png(G_filename) && pData && nDataLen > 0)
    {
        dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(G_filename), (const char *)pData, nDataLen);
    }
    return old_initWithImageData(self, pData, nDataLen, eFmt, nWidth, nHeight, nBitsPerComponent);
}

//bool Image::initWithImageData(const unsigned char * data, ssize_t dataLen)
HOOK_DEF(bool, initWithImageData_3x, void *self, const unsigned char * data, ssize_t dataLen) {
    DUALLOGD("[+] [%s] data[%p] len[%d] name[%s]", __FUNCTION__, data, dataLen, G_filename);
    if (strstr(G_filename, TEMP_PATH) && !check_png(G_filename) && data && dataLen > 0)
    {
        dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(G_filename), (const char *)data, dataLen);
    }
    return old_initWithImageData_3x(self, data, dataLen);
}

WALK_FUNC(Image) {
    if (check_lua(name))
        return;

    memset(G_filename, 0, sizeof(G_filename));
    snprintf(G_filename, sizeof(G_filename), "%s", name);

    DUALLOGD("filename[%s]", G_filename);

#if 1
    if (old_Sprite_create) old_Sprite_create(G_filename);
    if (old_Sprite_create_) old_Sprite_create_(G_filename);
    if (old_sharedTextureCache && old_addImage) old_addImage(old_sharedTextureCache(), G_filename);

    if (!old_initWithImageData_3x && !old_initWithImageData && buff && len > 0)
    {
        dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(G_filename), (const char *)buff, len);
    }
#else
    if (old_sharedTextureCache && old_addImage) old_addImage(old_sharedTextureCache(), G_filename);
#endif
}

typedef struct
{
    unsigned char* data;
    int size;
    int offset;
}tImageSource;

//png data
//void png_set_read_fn(png_structrp png_ptr, png_voidp io_ptr, png_rw_ptr read_data_fn)
HOOK_DEF(void, png_set_read_fn, void * png_ptr, tImageSource* io_ptr, void* read_data_fn) {
    if (io_ptr)
    {
        DUALLOGD("[+] [%s] data[%p] len[%d] name[%s]", __FUNCTION__, io_ptr->data, io_ptr->size, G_filename);
        if (strstr(G_filename, TEMP_PATH) && io_ptr->data && io_ptr->size > 0)
        {
            dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(G_filename), (const char *)io_ptr->data, io_ptr->size);
        }
    }
    return old_png_set_read_fn(png_ptr, io_ptr, read_data_fn);
}

//jpg jpeg data
//EXTERN(void) jpeg_mem_src JPP((j_decompress_ptr cinfo, unsigned char * inbuffer, unsigned long insize));
HOOK_DEF(void, jpeg_mem_src, void *cinfo, unsigned char * inbuffer, unsigned long insize) {
    DUALLOGD("[+] [%s] data[%p] len[%d] name[%s]", __FUNCTION__, inbuffer, insize, G_filename);
    if (strstr(G_filename, TEMP_PATH) && inbuffer && insize > 0)
    {
        dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(G_filename), (const char *)inbuffer, insize);
    }
    return old_jpeg_mem_src(cinfo, inbuffer, insize);
}

//pkm data
//GL_API void GL_APIENTRY glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
HOOK_DEF(void, glCompressedTexImage2D, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data) {
    DUALLOGD("[+] [%s] data[%p] len[%d] name[%s]", __FUNCTION__, data, imageSize, G_filename);
    if (strstr(G_filename, TEMP_PATH) && data && imageSize > 0)
    {
        dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(G_filename), (const char *)data, imageSize);
    }
    return old_glCompressedTexImage2D(target, level, internalformat,width, height, border, imageSize, data);
}

//bool CCTexture2D::initWithETCFile(const char* file)
HOOK_DEF(bool, initWithETCFile, void *self, const char* file) {
    DUALLOGD("[+] [%s] file[%s]", __FUNCTION__, file);
    return old_initWithETCFile(self, file);
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
    DUALLOGD("[+] [%s] G_lua_State[%p]", __FUNCTION__, L);
    int ret = old_cocos2dx_lua_loader(L);
    //if (G_lua_State && G_bDumpLua == 0) start_dump();
    return ret;
}

void *G_CCLuaStack = NULL;
HOOK_DEF(int, luaLoadBuffer, void *self, void* L, const char* chunk, int chunkSize, const char* chunkName) {
    G_CCLuaStack = self;
    //if (G_lua_State && G_bDumpLua == 0) start_dump();
    return old_luaLoadBuffer(self, L, chunk, chunkSize, chunkName);
}

WALK_FUNC(luaLoadBuffer) {
    if (!check_lua(name))
        return;

    if (G_lua_State) {
        DUALLOGD("[+] [%s] G_lua_State[%p] name[%s]", __FUNCTION__, G_lua_State, name);
#if 1
        old_lua_settop(G_lua_State, 0);
        old_lua_pushstring(G_lua_State, name);
        new_cocos2dx_lua_loader(G_lua_State);
#else
        if (new_luaLoadBuffer && G_CCLuaStack)new_luaLoadBuffer(G_CCLuaStack, G_lua_State, buff, len, name);
#endif
    }
    else
    {
        DUALLOGE("[-] [%s] G_lua_State[%p] not find .", __FUNCTION__, G_lua_State);
    }
}

//int (luaL_loadbuffer) (lua_State *L, const char *buff, size_t sz, const char *name);
HOOK_DEF(int, luaL_loadbuffer, void *L, const char *buff, size_t size, const char *name) {
    DUALLOGD("[+] [%s] L[%p] buff[%p] size[%d] name[%s]", __FUNCTION__, L, buff, size, name);

#if 0
    call_stack();
#endif

    if (G_HookConfig->dump_lua) {
        //if (buff && size > 0)
        if (strstr(name, TEMP_PATH) && buff && size > 0)
        {
            if (dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(name), buff, size) == 0)
                G_bDumpLua++;
        }
    }

    if (G_HookConfig->dump_inject) {
        void *out_buffer = NULL;
        size_t out_len = 0;
        if (replace_buffer(std::string(INJECT_PATH).append("/").append(PACK_NAME).c_str(), name, G_injectFiles, out_buffer, out_len) == 0) {
            DUALLOGD("注入lua[%s]成功", name);
        }
        if (out_buffer && out_len > 0) return old_luaL_loadbuffer(L, (const char *)out_buffer, out_len, name);
        return old_luaL_loadbuffer(L, buff, size, name);
    }
    else {
        return old_luaL_loadbuffer(L, buff, size, name);
    }
}

//LUALIB_API int luaL_loadbufferx (lua_State *L, const char *buff, size_t size, const char *name, const char *mode)
HOOK_DEF(int, luaL_loadbufferx, void *L, const char *buff, size_t size, const char *name, const char *mode) {
    DUALLOGD("[+] [%s] L[%p] buff[%s] size[%d] name[%s]", __FUNCTION__, L, buff, size, name);
    return old_luaL_loadbufferx(L, buff, size, name, mode);
}

typedef const char * (*lua_Reader) (void *L, void *ud, size_t *sz);
typedef struct LoadS {
    const char *s;
    size_t size;
} LoadS;
//LUA_API int   (lua_load) (lua_State *L, lua_Reader reader, void *dt, const char *chunkname, const char *mode);
HOOK_DEF(int, lua_load, void *L, lua_Reader reader, LoadS *dt, const char *chunkname, const char *mode) {
    DUALLOGD("[+] [%s] L[%p] buff[%s] size[%d] name[%s]", __FUNCTION__, L, dt->s, dt->size, chunkname);
    if (G_HookConfig->dump_lua) {
        //if (strstr(chunkname, TEMP_PATH) && dt->s && dt->size > 0)
            dump_write(PACK_NAME, ASSET_PATH, ASSET_NAME(chunkname), dt->s, dt->size);
    }
    return old_lua_load(L, reader, dt, chunkname, mode);
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

//int Application::run()
HOOK_DEF(int, Application_run, void *self) {
    int ret = old_Application_run(self);
    start_dump();
    return ret;
}

//#define SELL
void cocos_entry(const char *name, void *handle)
{
    //toast("cocos 开始注入...");
    G_Handle = handle;
    G_bWalkLuaCount = 0;
    G_bWalkResCount = 0;
    G_bDumpLua = 0;
    G_bDumpRes = 0;
    G_bWriteXXTEA = 0;
    G_injectFiles.clear();

    unsigned long base = 0;
    findLibBase(HOOK_NAME, &base);

    //HOOK启动函数 //_ZN7cocos2d13CCApplication3runEv
    if (!MS(handle, "_ZN7cocos2d11Application3runEv", Application_run))
        MS(handle, "_ZN7cocos2d13CCApplication3runEv", Application_run);

    if (G_HookConfig->dump_lua)
    {
        /* lua func */
        MS(handle, "lua_pushstring", lua_pushstring);
        MS(handle, "lua_settop", lua_settop);
        MS(handle, "luaL_openlibs", luaL_openlibs);
        if (!MS(handle, "luaL_loadbuffer", luaL_loadbuffer))
            MS(handle, "lua_load", lua_load);
        /* lua func */
        MS(handle, "cocos2dx_lua_loader", cocos2dx_lua_loader);
        //MS(handle, "_ZN7cocos2d10CCLuaStack13luaLoadBufferEP9lua_StatePKciS4_", luaLoadBuffer);

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
        //MS(handle, "jpeg_mem_src", jpeg_mem_src);
        //MS(handle, "png_set_read_fn", png_set_read_fn);
        //MS(handle, "glCompressedTexImage2D", glCompressedTexImage2D);
        MS_Function((void *)glCompressedTexImage2D, glCompressedTexImage2D);
#if 1
        if (!MS(handle, "_ZN7cocos2d7CCImage17initWithImageDataEPviNS0_12EImageFormatEiii", initWithImageData))
            MS(handle, "_ZN7cocos2d5Image17initWithImageDataEPKhi", initWithImageData_3x);
#endif
        if (!MS(handle, "_ZN7cocos2d6Sprite6createERKSs", Sprite_create))
            if (!MS(handle, "_ZN7cocos2d6Sprite6createEv", Sprite_create))
                MS(handle, "_ZN7cocos2d8CCSprite6createEPKc", Sprite_create_);

#if 1
        DUALLOGD("handle 0x[%x] base 0x[%x]", handle, base);
        MS_THUMB_SIMULATOR(base, 0x010F96FC, addImage);
        MS_THUMB_SIMULATOR(base, 0x010F7C20, sharedTextureCache);
        MS_THUMB_SIMULATOR(base, 0x010B109C, initWithImageData);
        //MS_THUMB_SIMULATOR(base, 0x010FBA28, initWithETCFile);
        //MS_THUMB(base, 0x0052F0CC, Sprite_create);
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