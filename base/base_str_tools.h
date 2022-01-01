/**
 * @file base_str_tools.h
 * @brief 主要包含各种string 处理的全局函数
 * @todo 将其抽象为基本的字符串工具类静态函数
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2020-12-15 23:57:25
 * @copyright Copyright (c) 2020  IRLSCU
 *
 * @par 修改日志:
 * <table>
 * <tr>
 *    <th> Commit date </th>
 *    <th> Version </th>
 *    <th> Author </th>
 *    <th> Description </th>
 * <tr>
 *    <td> 2020-12-15 23:57:25 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td>添加注释</td>
 * </tr>
 * </table>
 */
#ifndef BASE_STR_TOOLS_H
#define BASE_STR_TOOLS_H

#include <string>
#include "base_define.h"

/**
 * @brief  去开头空字符
 * @param  s               原始字符串
 * @return std::string&    输出字符串
 */
std::string &StringLTrimg(std::string &s);
/**
 * @brief  去除尾部空字符
 * @param  s               原始字符串
 * @return std::string&    输出字符串
 */
std::string &StringRTrim(std::string &s);
/**
 * @brief  去除首位空字符
 * @param  s               原始字符串
 * @return std::string&    输出字符串
 */
std::string &StringTrim(std::string &s);
/**
 * @brief  将字符串中的空字符串代替
 * @param  s                原始字符
 * @param  lookFor          目标字符串
 * @param  replaceWith      替换字符串
 * @return std::string&     处理结果
 */
// Replace sub-string within a string
std::string &StringReplace(std::string &s, const std::string &lookFor, const std::string &replaceWith);

#endif // BASE_STR_TOOLS_H