/**
 * 定义BaseManualSesetEvent类，方便管理信号和线程操作
 * 
*/

#ifndef BASE_MANUAL_RESET_EVENT_H
#define BASE_MANUAL_RESET_EVENT_H
#include <stdint.h>
#include <condition_variable>

#include "base_tool.h"

NAMESPACE_START

/*
定义基础事件数据结构
*/
class BaseManualResetEventData{
public:
        BaseManualResetEventData( ) :
            Counter( 0 ), Triggered( false ),
            Mutex( ), CondVariable( )
        {

        }
public:
    uint32_t                Counter;                /* 线程操作数目统计 */
    bool                    Triggered;              /* 是否发射信号变量 */
    std::mutex              Mutex;                  /* 变量访问互斥变量 */
    std::condition_variable CondVariable;           /* 统一信号变量 */
};

//手动重置同步事件
class BaseManualResetEvent
{
private:
    BaseManualResetEvent( const BaseManualResetEvent& );
    BaseManualResetEvent& operator= ( const BaseManualResetEvent& );

public:
    BaseManualResetEvent( );
    ~BaseManualResetEvent( );

    // 重设信号状态
    void Reset( );
    // 设置信号事件
    void Signal( );
    // 等待信号
    void Wait( );
    // 等待指定的时间（毫秒），直到事件得到通知
    bool Wait( uint32_t msec );
    // 检查事件的当前状态
    bool IsSignaled( );

private:
    BaseManualResetEventData* mData;
};

typedef BaseManualResetEvent ManualResetEvent;

NAMESPACE_END

#endif