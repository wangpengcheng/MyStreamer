/**
 * @file base_manual_reset_event.h
 * @brief 定义BaseManualSesetEvent类，方便管理信号和线程操作
 * @details 基础事件管理类，方便信号管理操作
 * @todo 将其和网络基础库中的 @link ::Condition合并功能  
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2020-12-15 16:05:43
 * @copyright Copyright (c) 2020  IRLSCU
 * 
 * @par 修改日志:
 * <table>
 * <tr>
 *    <th>Commit date</th>
 *    <th>Version</th> 
 *    <th>Author</th>  
 *    <th>Description</th>
 * <tr>
 *    <td> 2020-12-15 16:05:43 </td>
 *    <td>1.0 </td>
 *    <td>wangpengcheng    </td>
 *    <td>添加文档注释</td>
 * </tr>
 * </table>
 */
/**
 * 
 * 
*/

#ifndef BASE_MANUAL_RESET_EVENT_H
#define BASE_MANUAL_RESET_EVENT_H
#include <stdint.h>
#include <condition_variable>

#include "base_tool.h"

NAMESPACE_START
/**
 * @brief 基础数据BaseManualResetEventData 基础数据类
 */
class BaseManualResetEventData
{
public:
    BaseManualResetEventData() : Counter(0), Triggered(false),
                                 Mutex(), CondVariable()
    {
    }

public:
    uint32_t Counter;                     ///< 线程操作数目统计
    bool Triggered;                       ///< 是否发射信号变量
    std::mutex Mutex;                     ///< 变量访问互斥变量
    std::condition_variable CondVariable; ///< 统一信号变量
};

/**
 * @brief 事件响应封装类
 */
class BaseManualResetEvent
{
private:
    BaseManualResetEvent(const BaseManualResetEvent &);
    BaseManualResetEvent &operator=(const BaseManualResetEvent &);

public:
    /**
     * @brief Construct a new Base Manual Reset Event object
     */
    BaseManualResetEvent();
    /**
     * @brief Destroy the Base Manual Reset Event object
     */
    ~BaseManualResetEvent();

    /**
     * @brief 重设信号状态
     */
    void Reset();
    /**
     * @brief 设置信号事件
     */
    void Signal();
    /**
     * @brief 等待信号
     */
    void Wait();
    /**
     * @brief 等待指定的时间（毫秒），直到事件得到通知
     * @param  msec            毫秒数
     * @return true            设置成功
     * @return false            设置失败
     */
    bool Wait(uint32_t msec);
    /**
     * @brief 检查事件的当前状态，确认信号是否已经收到 
     * @return true     信号已经收到
     * @return false    信号还未收到
     */
    bool IsSignaled();

private:
    BaseManualResetEventData *mData; ///< 基础数据操作类
};

typedef BaseManualResetEvent ManualResetEvent;

NAMESPACE_END

#endif