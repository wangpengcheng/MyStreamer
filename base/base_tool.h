
/* 
    定义基本的使用工具 
*/
#ifndef BASE_TOOL_H
#define BASE_TOOL_H

#include "base_define.h"
#include <chrono>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/time.h>
#include <sstream>
#include <fstream>  
#include <streambuf>  

/* 获取当前的时间 */
std::time_t getTimeStamp();
/* 写入文件 */
void WriteFile(std::string file_name ,uint8_t* buffer,uint32_t buffer_size );
/* 获取当前时间 */
double cs_time(void);
/* 文件是否存在 */
bool FileExiting(const std::string& file_full_name);
/* 读取文件 */
std::string ReadFile(std::string file_full_name);
/* 字符串处理函数获取文件后缀名称 */
std::string GetFileType(const std::string& file_name);
#endif //BASE_TOOL_H