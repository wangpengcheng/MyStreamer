/**
 * @file net_callbacks.h
 * @brief 主要定义loop的时间回调函数
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2021-10-19 14:55:53
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
 *    <td> 2021-10-19 14:55:53 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td> 添加文档注释 </td>
 * </tr>
 * </table>
 */

#ifndef NET_CALLBACKS_H
#define NET_CALLBACKS_H
#include <functional>
#include <memory>
/* 添加时间戳工具 */
#include "time_stamp.h"

NAMESPACE_START

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

// should really belong to base/Types.h, but <memory> is not included there.

template <typename T>
inline T *get_pointer(const std::shared_ptr<T> &ptr)
{
    return ptr.get();
}

template <typename T>
inline T *get_pointer(const std::unique_ptr<T> &ptr)
{
    return ptr.get();
}

// Adapted from google-protobuf stubs/common.h
// see License in muduo/base/Types.h
template <typename To, typename From>
inline ::std::shared_ptr<To> down_pointer_cast(const ::std::shared_ptr<From> &f)
{
    if (false)
    {
        implicit_cast<From *, To *>(0);
    }

#ifndef NDEBUG
    assert(f == NULL || dynamic_cast<To *>(get_pointer(f)) != NULL);
#endif
    return ::std::static_pointer_cast<To>(f);
}
namespace net
{

    class Buffer;
    class TcpConnection;
    /* 自定义函数类别 */
    /* TCPl连接函数 */
    typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
    /* 时钟函数 */
    typedef std::function<void()> TimerCallback;
    /* 已连接时钟函数，主要用于主动发送 */
    typedef std::function<void(const TcpConnectionPtr &)> ConnectedTimerCallback;
    /* 连接函数 */
    typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
    /* 关闭函数 */
    typedef std::function<void(const TcpConnectionPtr &)> CloseCallback;
    /* 写入函数 */
    typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;
    /*  */
    typedef std::function<void(const TcpConnectionPtr &, size_t)> HighWaterMarkCallback;

    // the data has been read to (buf, len)
    typedef std::function<void(const TcpConnectionPtr &,
                               Buffer *,
                               Timestamp)>
        MessageCallback;
    /* TCP默认连接函数 */
    void defaultConnectionCallback(const TcpConnectionPtr &conn);
    /* 默认回调函数 */
    void defaultMessageCallback(const TcpConnectionPtr &conn,
                                Buffer *buffer,
                                Timestamp receiveTime);
} //namespace net

NAMESPACE_END

#endif