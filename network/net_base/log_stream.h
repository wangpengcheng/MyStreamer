/**
 * @file log_stream.h
 * @brief  日志流相关文件
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2021-12-05 00:13:08
 * @copyright Copyright (c) 2021  IRLSCU
 * 
 * @par 修改日志:
 * <table>
 * <tr>
 *    <th> Commit date</th>
 *    <th> Version </th> 
 *    <th> Author </th>  
 *    <th> Description </th>
 * </tr>
 * <tr>
 *    <td> 2021-12-05 00:13:08 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td> 添加文档注释 </td>
 * </tr>
 * </table>
 */

#ifndef BASE_LOGSTREAM_H
#define BASE_LOGSTREAM_H

#include "uncopyable.h"
#include "string_piece.h"
#include "base_types.h"

#include <assert.h>
#include <string.h> // memcpy
/**
 * 
 * 日志流信息
 * 主要用于重载流
 * **/
NAMESPACE_START
namespace detail
{
    /**
     * @brief 
     */
    const int kSmallBuffer = 4000;
    const int kLargeBuffer = 4000 * 1000;
    // buffer模板类
    template <int SIZE>
    class FixedBuffer : noncopyable
    {
    public:
        FixedBuffer()
            : cur_(data_)
        {
            setCookie(cookieStart);
        }

        ~FixedBuffer()
        {
            setCookie(cookieEnd);
        }

        void append(const char * /*restrict*/ buf, size_t len)
        {
            // FIXME: append partially
            if (implicit_cast<size_t>(avail()) > len)
            {
                memcpy(cur_, buf, len);
                cur_ += len;
            }
        }

        const char *data() const { return data_; }
        int length() const { return static_cast<int>(cur_ - data_); }

        // write to data_ directly
        char *current() { return cur_; }
        // 检测剩余的容量空间
        int avail() const { return static_cast<int>(end() - cur_); }
        // 添加
        void add(size_t len) { cur_ += len; }
        // 重置标志位
        void reset() { cur_ = data_; }
        // 将数组队列设置为0
        void bzero() { memZero(data_, sizeof(data_)); }

        // for used by GDB
        const char *debugString();
        void setCookie(void (*cookie)()) { cookie_ = cookie; }
        // for used by unit test
        string toString() const { return string(data_, length()); }
        StringPiece toStringPiece() const { return StringPiece(data_, length()); }

    private:
        const char *end() const { return data_ + sizeof(data_); }
        // Must be outline function for cookies.
        static void cookieStart();
        static void cookieEnd();

        void (*cookie_)();      /*cookie_函数指针*/
        char data_[SIZE];       /* 数据的大小 */
        char *cur_;             /* 当前的数据指针 */
    };

} // namespace detail
/**
 * 小型日志缓冲类
 * 
 * **/
class LogStream : noncopyable
{
    typedef LogStream self;

public:
    typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;

    self &operator<<(bool v)
    {
        buffer_.append(v ? "1" : "0", 1);
        return *this;
    }

    self &operator<<(short);
    self &operator<<(unsigned short);
    self &operator<<(int);
    self &operator<<(unsigned int);
    self &operator<<(long);
    self &operator<<(unsigned long);
    self &operator<<(long long);
    self &operator<<(unsigned long long);

    self &operator<<(const void *);

    self &operator<<(float v)
    {
        *this << static_cast<double>(v);
        return *this;
    }
    self &operator<<(double);
    // self& operator<<(long double);

    self &operator<<(char v)
    {
        buffer_.append(&v, 1);
        return *this;
    }

    // self& operator<<(signed char);
    // self& operator<<(unsigned char);

    self &operator<<(const char *str)
    {
        if (str)
        {
            buffer_.append(str, strlen(str));
        }
        else
        {
            buffer_.append("(null)", 6);
        }
        return *this;
    }

    self &operator<<(const unsigned char *str)
    {
        return operator<<(reinterpret_cast<const char *>(str));
    }

    self &operator<<(const string &v)
    {
        buffer_.append(v.c_str(), v.size());
        return *this;
    }

    self &operator<<(const StringPiece &v)
    {
        buffer_.append(v.data(), v.size());
        return *this;
    }

    self &operator<<(const Buffer &v)
    {
        *this << v.toStringPiece();
        return *this;
    }

    void append(const char *data, int len) { buffer_.append(data, len); }
    const Buffer &buffer() const { return buffer_; }
    void resetBuffer() { buffer_.reset(); }

private:
    void staticCheck();

    template <typename T>
    void formatInteger(T);

    Buffer buffer_;

    static const int kMaxNumericSize = 32;
};

class Fmt // : noncopyable
{
public:
    template <typename T>
    Fmt(const char *fmt, T val);

    const char *data() const { return buf_; }
    int length() const { return length_; }

private:
    char buf_[32];
    int length_;
};
/**
 * 全局重载流变化
 * **/
inline LogStream &operator<<(LogStream &s, const Fmt &fmt)
{
    s.append(fmt.data(), fmt.length());
    return s;
}

// Format quantity n in SI units (k, M, G, T, P, E).
// The returned string is atmost 5 characters long.
// Requires n >= 0
string formatSI(int64_t n);

// Format quantity n in IEC (binary) units (Ki, Mi, Gi, Ti, Pi, Ei).
// The returned string is atmost 6 characters long.
// Requires n >= 0
string formatIEC(int64_t n);

NAMESPACE_END

#endif // BASE_LOGSTREAM_H
