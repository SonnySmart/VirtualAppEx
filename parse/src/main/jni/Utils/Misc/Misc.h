//
// Created by Apple on 2019/4/9.
//

#ifndef __UTILS_MISC__
#define __UTILS_MISC__

#include <vector>
#include <string>

typedef void (*ptr_read_buffer)(char *name, char *buff, size_t len, void *param);

// read string
const std::string &read_string(const char *filename);
// read buffer
void read_buffer(const char *filename, char *&buff, size_t &out_len);
// 遍历文件夹
void dirwalk(const std::string &dir_name, std::vector<std::string> &files);
// get all files buffer
void filewalk(const char *name, ptr_read_buffer ptr_read, void *param, size_t &walk_count, bool read_buff = true);
// mkdir
void mkdirs(const std::string &dir);
// dump lua
int dump_write(const char *pack, const char *path, const char *name, const char *buff, const size_t &len);
// replace lua
int replace_buffer(const char *root, const char *name, const std::vector<std::string> &r, void *&out_buffer, size_t &out_len);
// 获取存储asset路径
const char *assets_name(const char *name, const char *tmp_path);
// ms_hook封装
bool ms_hook(void *handle, const char *symbol, void *replace, void **result);

#endif //__UTILS_MISC__
