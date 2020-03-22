//
// Created by Apple on 2019/4/9.
//

#include "Misc.h"
#include "../Includes.h"
#include <string>
#include <malloc.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <vector>

void read_string(const char *filename, std::string &ret)
{
    FILE *fp;
    if (!(fp = fopen(filename, "r"))) {
        DUALLOGE("[-] [%s] file not exist [%s]", __FUNCTION__, filename);
        return;
    }

    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    void *buff = malloc(len);
    if (fread(buff, 1u, len, fp) <= 0)
        return;

    ret = (char *)buff;
    free(buff);
}

void read_buffer(const char *filename, char *&buff, size_t &out_len)
{
    if (access(filename, F_OK) != 0)
        return;

    FILE *fd = NULL;
    if ((fd = fopen(filename, "rb"))) {
        fseek(fd, 0, SEEK_END);
        size_t len = ftell(fd);
        if (len <= 0) {
            fclose(fd);
            DUALLOGE("[-][%s] file len <= 0", __FUNCTION__);
            return;
        }
        fseek(fd, 0, SEEK_SET);
        char *buffer = (char *)malloc(len);

        if (fread(buffer, 1u, len, fd) <= 0) {
            fclose(fd);
            DUALLOGE("[-][%s] file fread <= 0", __FUNCTION__);
            return;
        }

        buff = buffer;
        out_len = len;

        fclose(fd);
    }
}

void dirwalk(const std::string &dir_name, std::vector<std::string> &files)
{
    if (dir_name.empty()) {
        DUALLOGE("dir_name is null !");
        return;
    }
    DIR *dir = opendir(dir_name.c_str());
    // check is dir ?
    if (NULL == dir) {
        DUALLOGE("Can not open dir. Check path or permission!");
        return;
    }
    struct dirent *file;
    // read all the files in dir
    while ((file = readdir(dir)) != NULL) {
        // skip "." and ".."
        if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0) {
            //LOGV("ignore . and ..");
            continue;
        }
        std::string filePath = dir_name + "/" + file->d_name;
        if (file->d_type == DT_DIR) {
            dirwalk(filePath, files); // 递归执行
        } else {
            // 如果需要把路径保存到集合中去，就在这里执行 add 的操作
            //LOGI("filePath: %s/%s", dir_name.c_str(), file->d_name);
            files.push_back(filePath);
        }
    }
    closedir(dir);
}

void filewalk(const char *name, ptr_read_buffer ptr_read, void *param, size_t &walk_count, bool read_buff)
{
    size_t index = 0;
    size_t len = 0;
    char *buff = NULL;
    char fullname[1024] = { 0 };
    char info [1024] = { 0 };
    FILE *fp = NULL;

    // 只会进行一次遍历
    if (walk_count++ > 0)
        return;

    std::vector<std::string> files;

    dirwalk(name, files);

    fp = fopen(LOG_FILE, "w");

    for(auto i = files.begin(); i != files.end(); ++i) {
        const char *filename = (*i).c_str();
        ++index;

        // 拼接信息
        memset(fullname, 0, sizeof(fullname));
        memset(info, 0, sizeof(info));
        snprintf(fullname, sizeof(fullname), "%s/%s/%s", ASSET_PATH, PACK_NAME, ASSET_NAME(filename));
        snprintf(info, sizeof(info), "[+] [%s] 进度[%d/%d] 文件[%s]\n", __FUNCTION__, index, files.size(), filename);
        // 打印日志
        DUALLOGD("[+] [%s] 进度[%d/%d] 文件[%s]", __FUNCTION__, index, files.size(), filename);
        // 写入日志
        if (fp) {
            if (fwrite(info, sizeof(char), strlen(info), fp) <= 0) {
                DUALLOGE("[-] [%s] errno[%s]", __FUNCTION__, errno);
            }
            fflush(fp);
        }

        // 文件存在就跳过
        if (access(fullname, F_OK) == 0)
            continue;

        // 不读取buffer只取路径
        if (!read_buff && ptr_read)
        {
            ptr_read((char *)filename, buff, len, param);
            continue;
        }
        // 读取路径和buffer
        read_buffer(filename, buff, len);
        if (buff && len > 0 && ptr_read) {
            ptr_read((char *)filename, buff, len, param);
        }
        else {
            DUALLOGE("[-] [%s] read_buffer buff[%p] len[%d]", __FUNCTION__, buff, len);
        }

        if (buff) free(buff);
    }

    if (fp) {
        memset(info, 0, sizeof(info));
        snprintf(info, sizeof(info), "[+] [%s] 遍历完成文件个数[%d]\n", __FUNCTION__, files.size());
        if (fwrite(info, sizeof(char), strlen(info), fp) <= 0) {
            DUALLOGE("[-] [%s] errno[%s]", __FUNCTION__, errno);
        }
        fflush(fp);
    }

    if (fp) fclose(fp);

    DUALLOGD("[+] [%s] 遍历完成文件个数[%d]", __FUNCTION__, files.size());
}

void mkdirs(const std::string &dir)
{
    int i,len;
    len = dir.length();
    char *str = (char *)malloc(len);
    strcpy(str, dir.c_str());
    for( i=0; i<len; i++ )
    {
        if( str[i]=='/' )
        {
            str[i] = '\0';
            if( access(str,0)!=0 )
            {
                mkdir( str, 0777 );
            }
            str[i]='/';
        }
    }
    if( len>0 && access(str,0)!=0 )
    {
        mkdir( str, 0777 );
    }
    free(str);
}

/* 从内存镜像中写入到文件,通常获得了解密后的文件
 * 文件保存在默认的路径"/sdcard/myhook/"目录下
 * 创建对应的包名文件夹,然后再对应目录下写入文件,其总目录中包含日志
 * 这里的name是绝对路径如:/data/app/com.game.sgz.uc-1.apk/assets/bin/Data/Managed/UnityEngine.dll,所以需要提取文件名
 * 名字不能为空,避免非法内存访问,还有len度为0的情况都应考虑
 * 考虑到写入的文件没有记录完整的路径因此在目录下添加日志来记录
 */
int dump_write(const char *pack, const char *path, const char *name, const char *buff, const size_t &len)
{
    if (!pack || !path || !buff || len <= 0)
        return -1;

    std::string filename(name);
    // 空名称转换为0xffffff-100文件名
    if (filename.empty())
    {
        char tmp[32] = { 0 };
        snprintf(tmp, sizeof(tmp), "%p-%d", buff, len);
        filename.append(tmp);
    }

    // 查找文件后缀
    std::string::size_type npos = filename.find_last_of('.');
    if (npos == std::string::npos)
        return -1;

    // 获取文件后缀
    std::string suffixStr = filename.substr(npos + 1);
    // luac替换为lua
    if (suffixStr == "luac")
        filename = filename.substr(0, npos).append(".lua");

    //if ((npos = filename.find(".apk/")) != std::string::npos)
        //filename = filename.substr(npos + 5);

    // /sdcard/myhook/tmp + packagename + filename全路径
    std::string full(path);
    full.append("/").append(pack).append("/").append(filename);
    const char *fullname = full.c_str();
    // 文件存在就返回不覆盖操作
    if (access(fullname, F_OK) == 0)
        return -2;

    // 检测文件是否有没创建的文件夹
    std::size_t i = full.find_last_of("/");
    if (i != std::string::npos)
    {
        std::string dir = full.substr(0, i);
        //DUALLOGD("dir[%s]", dir.c_str());
        if (access(dir.c_str(), F_OK) != 0) {
            mkdirs(dir);
        }
    }

    // 写入文件
    FILE *fd = fopen(fullname, "wb");
    if (fd) {
        if (fwrite(buff, len, 1u, fd) > 0) {
            DUALLOGW("[+] [%s] fwrite:%s", __FUNCTION__, fullname);
            fclose(fd);
            return 0;
        }
        else {
            DUALLOGE("[-] [%s] fwrite failed:%s errno[%s]", __FUNCTION__, fullname, errno);
        }
        fclose(fd);
    }
    else {
        DUALLOGE("[-][%s] fopen failed:%s errno[%s]", __FUNCTION__, fullname, errno);
    }

    return -4;
}

int replace_buffer(const char *root, const char *name, std::vector<std::string> &r, void *&out_buffer, size_t &out_len)
{
    if (strlen(root) == 0 || strlen(name) == 0)
        return -1;

    if (r.size() == 0) dirwalk(root, r);

    for(auto i = r.begin(); i != r.end(); ++i) {
        const char *fullpath = (*i).c_str();
        const char *filename = assets_name(fullpath, root);
        //DUALLOGD("[+] filename[%s]", filename.c_str());
        //文件不存在 | 文件不匹配
        if (access(fullpath, F_OK) != 0 || strstr(name, filename) == NULL)
            continue;

        FILE *fd = NULL;
        if ((fd = fopen(fullpath, "rb"))) {
            fseek(fd, 0, SEEK_END);
            size_t len = ftell(fd);
            if (len <= 0) {
                fclose(fd);
                DUALLOGE("[-][%s] file len <= 0", __FUNCTION__);
                continue;
            }
            fseek(fd, 0, SEEK_SET);
            void *buffer = malloc(len);

            if (fread(buffer, 1u, len, fd) <= 0) {
                fclose(fd);
                DUALLOGE("[-][%s] file fread <= 0", __FUNCTION__);
                continue;
            }

            DUALLOGI("[+][%s] read buffer:%x", __FUNCTION__, buffer);

            out_buffer = buffer;
            out_len = len;

            fclose(fd);

            return 0;
        }
    }

    return -1;
}

const char *assets_name(const char *name, const char *tmp_path)
{
    std::string filename = name;
    if (filename.empty())
        return "";
    if (strstr(name, tmp_path))
        filename = filename.substr(strlen(tmp_path), filename.length() - strlen(tmp_path));
    return filename.c_str();
}

bool inline_hook(void *handle, const char *symbol_name, void *replace, void **result)
{
    if (!handle)
    {
        DUALLOGE("[-] [%s] handle[%p]", __FUNCTION__, handle);
        return false;
    }

    void *symbol = dlsym(handle, symbol_name);
    if (!symbol)
    {
        DUALLOGE("[-] [%s] symbol[%s] dlerror[%s]", __FUNCTION__, symbol_name, dlerror());
        return false;
    }

#if WHALE
    G_WInlineHookFunction(symbol, replace, result);
#elif ANDHOOK
    G_WInlineHookFunction(symbol, replace, result);
#else
    MSHookFunction(symbol, replace, result);
#endif

    DUALLOGI("[+] [%s] symbol_name[%s] symbol[%p]", __FUNCTION__, symbol_name, symbol);

    return true;
}

int findSymbol(const char *name, const char *libn, unsigned long *addr) {
    return find_name(getpid(), name, libn, addr);
}

int findLibBase(const char *libn, unsigned long *addr) {
    return find_libbase(getpid(), libn, addr);
}