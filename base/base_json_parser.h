/**
 * 定义json字符串解析函数
*/
#ifndef BASE_JSON_PARSER_H
#define BASE_JSON_PARSER_H
#include "base_tool.h"
#include <string>
#include <map>
/* 
 * 注意只能解析，简单的json文本，不能解析复杂的嵌套json 
 * https://blog.csdn.net/liitdar/article/details/80522415
 * https://github.com/open-source-parsers/jsoncpp
 * ToDo:使用JSONCPP实现JSON文件的解析
 * 
*/

bool SimpleJsonParser( const std::string& jsonStr, std::map<std::string, std::string>& values );



#endif