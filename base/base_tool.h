
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
/* 获取当前的时间 */
inline std::time_t getTimeStamp()
{
    std::chrono::time_point<std::chrono::system_clock,std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    auto tmp=std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
    std::time_t timestamp = tmp.count();
    return timestamp;
}

inline void WriteFile(std::string file_name ,uint8_t* buffer,uint32_t buffer_size )
{
    std::ofstream file_out;
    file_out.open(file_name,std::ios_base::binary|std::ios_base::out);
    if(file_out.is_open()){
        file_out.write((char*)buffer,buffer_size);
    }else{
        std::cout<<"file open error"<<std::endl;
    }
    
}
#endif //BASE_TOOL_H