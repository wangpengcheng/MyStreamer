/**
 * @file base_json_parser.h
 * @brief 简单json解析文件
 * @details 注意只能解析，简单的json文本，不能解析复杂的嵌套json \n
 * 详情参考：<a href= "https://blog.csdn.net/liitdar/article/details/80522415">JSON解析（C++）</a>;
 *          <a href="https://github.com/open-source-parsers/jsoncpp">jsoncpp源码</a>
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2020-12-15 12:45:17
 * @copyright Copyright (c) 2020  IRLSCU
 * @todo 封装JSONCPP 为抽象类，使用JSONCPP实现JSON文件的解析
 * @par 修改日志:
 * <table>
 * <tr>
 *    <th>Commit date</th>
 *    <th>Version</th> 
 *    <th>Author</th>  
 *    <th>Description</th>
 * <tr>
 *    <td>2020-12-15 12:45:17 </td>
 *    <td>1.0 </td>
 *    <td>wangpengcheng    </td>
 *    <td>完成文档补充</td>
 * </tr>
 * </table>
 */
#ifndef BASE_JSON_PARSER_H
#define BASE_JSON_PARSER_H
#include "base_tool.h"
#include <string>
#include <map>
/**
 * @brief  简单的json文件解析函数，用来实现简单的json 脚本解析
 * @param  jsonStr          原始的json字符串
 * @param  values           解析结果输出Map函数值
 * @return true             解析成功
 * @return false            解析失败
 * 
 */
bool SimpleJsonParser( const std::string& jsonStr, std::map<std::string, std::string>& values );



#endif