
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

/**
 * @brief  获取当前的时间 
 * @return std::time_t unix 时间结构体
 */
std::time_t getTimeStamp();
/**
 * @brief  写入文件 
 * @param  file_name        文件名称
 * @param  buffer           缓冲数据指针
 * @param  buffer_size      缓冲区大小
 */
void WriteFile(std::string file_name, uint8_t *buffer, uint32_t buffer_size);
/**
 * @brief  获取当前时间 
 * @return double      时间结果
 */
double cs_time(void);
/**
 * @brief  检查文件是否存在 
 * @param  file_full_name   全路径文件名称
 * @return true             文件存在
 * @return false            文件不存在
 */
bool FileExiting(const std::string &file_full_name);
/* 读取文件 */
/**
 * @brief  读取文件成字符串
 * @param  file_full_name   全路径文件名称
 * @return std::string      文件最终读取结果
 */
std::string ReadFile(std::string file_full_name);
/**
 * @brief 字符串处理函数获取文件后缀名称
 * @param  file_name        文件名称
 * @return std::string      最终结果
 */
std::string GetFileType(const std::string &file_name);
#endif //BASE_TOOL_H