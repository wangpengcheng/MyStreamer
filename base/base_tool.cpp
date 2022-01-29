#include "base_tool.h"

/**
 * 
 * 获取当前的时间戳字符串
 * */
std::time_t getTimeStamp()
{
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
    std::time_t timestamp = tmp.count();
    return timestamp;
};

void WriteFile(std::string file_name, uint8_t *buffer, uint32_t buffer_size)
{
    std::ofstream file_out;
    file_out.open(file_name, std::ios_base::binary | std::ios_base::out);
    if (file_out.is_open())
    {
        file_out.write((char *)buffer, buffer_size);
    }
    else
    {
        std::cout << "file open error" << std::endl;
    }
};
double cs_time(void)
{
    double now;
#ifndef _WIN32
    struct timeval tv;
    if (gettimeofday(&tv, NULL /* tz */) != 0)
        return 0;
    now = (double)tv.tv_sec + (((double)tv.tv_usec) / 1000000.0);
#else
    SYSTEMTIME sysnow;
    FILETIME ftime;
    GetLocalTime(&sysnow);
    SystemTimeToFileTime(&sysnow, &ftime);
    /*
   * 1. VC 6.0 doesn't support conversion uint64 -> double, so, using int64
   * This should not cause a problems in this (21th) century
   * 2. Windows FILETIME is a number of 100-nanosecond intervals since January
   * 1, 1601 while time_t is a number of _seconds_ since January 1, 1970 UTC,
   * thus, we need to convert to seconds and adjust amount (subtract 11644473600
   * seconds)
   */
    now = (double)(((int64_t)ftime.dwLowDateTime +
                    ((int64_t)ftime.dwHighDateTime << 32)) /
                   10000000.0) -
          11644473600;
#endif /* _WIN32 */
    return now;
};
bool FileExiting(const std::string &file_full_name)
{
    std::ifstream fin(file_full_name);
    return fin.good();
};
std::string ReadFile(std::string file_full_name)
{
    std::ifstream fin(file_full_name);
    /* 检查文件是否存在 */
    if (!fin.is_open())
    {
        return "";
    }
    else
    {
        std::ostringstream tmp;
        tmp << fin.rdbuf();
        std::string str = tmp.str();
        return str;
    }
};
/* 字符串处理函数获取文件后缀名称 */
std::string GetFileType(const std::string &file_name)
{
    std::string::size_type n;
    n = file_name.rfind(".");
    if (n == std::string::npos || n == 0)
    {
        return "";
    }
    else
    {
        return file_name.substr(n);
    }
};