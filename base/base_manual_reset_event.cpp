#include "base_manual_reset_event.h"

NAMESPACE_START

BaseManualResetEvent::BaseManualResetEvent( ) :
    mData( new BaseManualResetEventData( ) )
{
}

BaseManualResetEvent::~BaseManualResetEvent( )
{
    delete mData;
}

// Set event to not signalled state
void BaseManualResetEvent::Reset( )
{
    std::unique_lock<std::mutex> lock( mData->Mutex );
    mData->Triggered = false;
}

void BaseManualResetEvent::Signal( )
{
    // 注意这里使用unique_lock，因为只有它能和condition_variable搭配使用
    std::unique_lock<std::mutex> lock( mData->Mutex );
    mData->Triggered = true;
    mData->Counter++;
    mData->CondVariable.notify_all( );
}

// 等待直到信号到达之前自旋
void BaseManualResetEvent::Wait( )
{
    std::unique_lock<std::mutex> lock( mData->Mutex );
    uint32_t           lastCounterValue = mData->Counter;

    while ( ( !mData->Triggered ) && ( mData->Counter == lastCounterValue ) )
    {
        mData->CondVariable.wait(lock);
    }
}

// 等待一定时间内，有信号到达；返回是否到已经更改
bool BaseManualResetEvent::Wait( uint32_t msec )
{
    std::chrono::steady_clock::time_point waitTill = std::chrono::steady_clock::now( ) + std::chrono::milliseconds( msec );
    std::unique_lock<std::mutex>       lock( mData->Mutex );
    /* 上一次统计值 */
    uint32_t        lastCounterValue = mData->Counter;

    if ( !mData->Triggered )
    {
        mData->CondVariable.wait_until( lock, waitTill );
    }
    /* 状态改变或者值改变，表示已经有信号叨叨 */
    return ( ( mData->Triggered ) || ( mData->Counter != lastCounterValue ) );
}

// 确认当前事件状态
bool BaseManualResetEvent::IsSignaled()
{
    std::unique_lock<std::mutex> lock( mData->Mutex );
    return mData->Triggered;
}



NAMESPACE_END
