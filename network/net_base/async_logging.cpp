// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "async_logging.h"
#include "log_file.h"
#include "time_stamp.h"
#include <stdio.h>

NAMESPACE_START

AsyncLogging::AsyncLogging(const string &basename,
                           off_t rollSize,
                           int flushInterval)
    : flushInterval_(flushInterval),
      running_(false),
      basename_(basename),
      rollSize_(rollSize),
      thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
      latch_(1),
      mutex_(),
      cond_(mutex_),
      currentBuffer_(new Buffer),
      nextBuffer_(new Buffer),
      buffers_()
{
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
}

void AsyncLogging::append(const char *logline, int len)
{
    MutexLockGuard lock(mutex_);
    if (currentBuffer_->avail() > len)
    {
        currentBuffer_->append(logline, len);
    }
    else
    {
        buffers_.push_back(std::move(currentBuffer_));

        if (nextBuffer_)
        {
            currentBuffer_ = std::move(nextBuffer_);
        }
        else
        {
            currentBuffer_.reset(new Buffer); // Rarely happens
        }
        currentBuffer_->append(logline, len);
        cond_.notify();
    }
}

void AsyncLogging::threadFunc()
{
    assert(running_ == true);
    latch_.countDown();
    LogFile output(basename_, rollSize_, false);
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite;
    // 扩充大小到16
    buffersToWrite.reserve(16);
    while (running_)
    {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());

        {
            MutexLockGuard lock(mutex_);
            // 缓冲区为空，则等待一段时间
            if (buffers_.empty()) // unusual usage!
            {
                cond_.waitForSeconds(flushInterval_);
            }
            // 将当前缓冲区添加到数据中
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(newBuffer1);
            // 释放空间
            buffersToWrite.swap(buffers_);
            if (!nextBuffer_)
            {
                nextBuffer_ = std::move(newBuffer2);
            }
        }

        assert(!buffersToWrite.empty());
        // 如果大小大于25；打一个日志断点
        if (buffersToWrite.size() > 25)
        {
            // 日志输出
            char buf[256];
            // 创建输入文字
            snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
                     Timestamp::now().toFormattedString().c_str(),
                     buffersToWrite.size() - 2);
            fputs(buf, stderr);
            // 将数据写入到文件中
            output.append(buf, static_cast<int>(strlen(buf)));
            // 删除并释放文件
            buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
        }
        // 循环进行写入
        for (const auto &buffer : buffersToWrite)
        {
            // FIXME: use unbuffered stdio FILE ? or use ::writev ?
            output.append(buffer->data(), buffer->length());
        }
        // 销毁
        if (buffersToWrite.size() > 2)
        {
            // drop non-bzero-ed buffers, avoid trashing
            buffersToWrite.resize(2);
        }

        if (!newBuffer1)
        {
            assert(!buffersToWrite.empty());
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if (!newBuffer2)
        {
            assert(!buffersToWrite.empty());
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear();
        output.flush();
    }
    output.flush();
}

NAMESPACE_END