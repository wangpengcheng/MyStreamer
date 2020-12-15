/**
 * @file base_ring_buffer.h
 * @brief ringbuffer环形接口缓冲区相关类
 * @details 定义ringbuffer方便使用，basebuffer使用模板进行初始化 \n
 * 定义通用一般接口，并对char进行特例化，增加write和read函数。 \n
 * buffer为多线程安全 \n
 * @todo 和  @link ::net::Buffer net::Buffer 功能存在重叠，需要进行融合，并统一接口
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2020-12-15 19:18:50
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
 *    <td>2020-12-15 19:18:50 </td>
 *    <td>1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td>添加文档注释</td>
 * </tr>
 * </table>
 */
#ifndef BASE_RING_BUFFER_H
#define BASE_RING_BUFFER_H

#include <vector>
#include <memory>
#include <atomic>
#include <iostream>
#include <ctime>
#include <cstring>
#include <string>
#include <stdint.h>
#include <stddef.h>
#include "base_tool.h"

NAMESPACE_START

/**
 * @brief 设置默认的环形缓冲区的大小
 */
#define DEFAULT_SIZE 30
/**
 * @brief 环形缓冲区
 * @details 简单的环形缓冲区模板 \n
 * <i>参考链接:</i>
 * <a href="https://blog.csdn.net/weixin_40825228/article/details/80783860?depth_1.utm_source=distribute.pc_relevant.none-task&utm_source=distribute.pc_relevant.none-task">无锁数据结构之——线程安全的SPSC队列</a> \n
 * 类似结构如下: \n
 * @code
 *   +-------------------+------------------+------------------+
 *   | prependable bytes |  readable bytes  |  writable bytes  |
 *   |                   |     (CONTENT)    |                  |
 *   +-------------------+------------------+------------------+
 *   |                   |                  |                  |
 *   0      <=      readerIndex   <=   writerIndex    <=     size
 * @endcode 
 * @attention 
 * 这只是一个简单的环形缓冲区模板，不具有多线程安全性 \n
 * 需要多线程安全，请使用Buffer  \n
 * 单读单写安全请, 请使用SingleRingBuffer 
 * @see ::net::Buffer \n
 *      ::SingleRingBuffer
 * \n
 * @tparam T 基础模板变量，一般都是char以字节为单位
 */
template <typename T>
class CircuBuffer
{
public:
    /**
     * @brief Construct a new Circu Buffer object 基础构造函数
     * @param  size             以模板开始的大小
     */
    CircuBuffer(unsigned size) : size_(size)
    {
        writeIndex_ = 0;
        readIndex_ = 0;
        buffer_ = new T[size_];
    }
    /**
     * @brief 空值构造函数；默认缓冲区大小为 DEFAULT_SIZE 
     * @see ::DEFAULT_SIZE
     */
    CircuBuffer() : size_(DEFAULT_SIZE)
    {
        writeIndex_ = 0;
        readIndex_ = 0;
        buffer_ = new T[size_];
    }
    /**
     * @brief Destroy the Circu Buffer object
     */
    ~CircuBuffer()
    {
        delete[] buffer_;
    }
    /**
     * @brief 获取当前readindex的位置，辅助式获取对应的剩余空间
     * @return unsigned 
     */
    unsigned getReadIndex() const
    {
        return readIndex_;
    }
    /**
     * @brief 获取当前可写index的位置，辅助获取对应的可读或者可写空间
     * @return unsigned 
     */
    unsigned getWriteIndex() const
    {
        return writeIndex_;
    }
    /**
     * @brief 检查缓冲区是否为空
     * @attention 当writeIndex_ == readIndex_时缓冲区为空 \n
     * 表示没有数据
     * @return true 缓冲区为空
     * @return false  缓冲区不为空
     */
    bool isEmpty()
    {
        return writeIndex_ == readIndex_;
    }
    /**
     * @brief 检查缓冲区是否已经满
     * @attention 
     * 当readIndex与writeIndex相邻时，表示缓冲区已满 \n
     * 因为两个index都是一直向前移动的，中间的差值才是 \n
     * 真正的数据存区域，移动方向相同，相邻表示缓冲区已满 \n
     * 相同表示数据已经被读空。
     * @return true 缓冲区为空
     * @return false 缓冲区不为空
     */
    bool isFull()
    {
        return (readIndex_ + 1) % size_ == writeIndex_;
    }
    /**
     * @brief  push添加一个元素
     * @param  element          元素
     * @return true             添加成功
     * @return false            添加失败
     */
    bool pushAnElement(T element)
    {
        //队列不为满
        if (!isFull())
        {
            buffer_[writeIndex_] = element;
            // 注意这里移动可写索引
            writeIndex_ = (writeIndex_ + 1) % size_;
            return true;
        }
        else
        {
            return false;
        }
    };
    /**
     * @brief 从缓冲区中获取一个元素
     * @return T* 返回元素对象
     */
    T *getAnElement()
    {
        //队列不为空
        if (!isEmpty())
        {
            T *temp = buffer_ + readIndex_;
            // 注意这里移动可读索引
            readIndex_ = (readIndex_ + 1) % size_;
            return temp;
        }
        else
        {
            return nullptr;
        }
    }

private:
    unsigned writeIndex_; ///< 可写指针
    unsigned readIndex_;  ///< 可读指针
    T *buffer_;           ///< buffer数据
    unsigned size_;       ///< 缓冲区大小
};
/**
 * @brief 基础单读单写ring buffer,使用内存序和原子操作，降低锁的粒度数据有序性
 * @details 获取获取更多信息，请阅读 <a href="https://www.codeproject.com/Articles/43510/Lock-Free-Single-Producer-Single-Consumer-Circular#_articleTop"> Lock-Free Single-Producer-Single Consumer Circular Queue </a>
 */
template <typename T, size_t Size>
class BaseSimpleRingBuffer
{
public:
    /**
     * @brief 基础构造函数
     * head和tail在开始时，初始化为0
     */
    BaseSimpleRingBuffer() : head_(0), tail_(0) {}
    bool push(const T &value)
    {
        size_t head = head_.load(std::memory_order_relaxed);
        size_t next_head = next(head);
        if (next_head == tail_.load(std::memory_order_acquire))
        {
            return false;
        }
        /* 在这里进行数据拷贝 */
        ring_[head] = value;
        head_.store(next_head, std::memory_order_release);
        return true;
    }
    /**
     * @brief  取出数据
     * @param  value            模板数据引用类型
     * @return true             取出数据成功
     * @return false            取出数据失败
     */
    bool pop(T &value)
    {
        size_t tail = tail_.load(std::memory_order_relaxed);
        /* 相等表示队列已满直接返回false */
        if (tail == head_.load(std::memory_order_acquire))
        {
            return false;
        }
        value = ring_[tail];
        /* 这里释放信号，保证先存后取 */
        tail_.store(next(tail), std::memory_order_release);
        return true;
    }

private:
    /**
     * @brief  计算机写一个的对应索引
     * @param  current          数据索引
     * @return size_t           对应的索引数据
     */
    size_t next(size_t current)
    {
        return (current + 1) % Size;
    }

    T ring_[Size];                  ///< 内部数据数组
    std::atomic<size_t> head_;      ///< 头部指针
    std::atomic<size_t> tail_;      ///< 尾部指针
};

/**
 * @brief  判断n是否为2的幂 
 * @param  n                输入参数
 * @return true             是2的幂
 * @return false            不是2的幂
 */
static bool is_power_of_2(unsigned int n)
{
    return (n != 0 && ((n & (n - 1)) == 0));
}

/**
 * @brief  将数字a向上取整为2的次幂
 * @param  a                输入数字
 * @return uint32_t         输出向上取整结果
 */
static uint32_t roundup_power_of_2(uint32_t a)
{
    if (a == 0){
        return 0;
    }
    uint32_t position = 0;
    for (int i = a; i != 0; i >>= 1) {
        position++;
    }
    return (uint32_t)(1 << position);
};

/**
 * 这里仿照kbuffer实现一个一读一写的高速无锁ringbuffer;
 * 注意这里的大小是2的整数倍大小，建议使用大小为2的整数倍，避免大量空间浪费
 * */
/**
 * 参考kfifo的basebufferdata基础数据管理类，
 * 注意这里没有加锁，因此是属于单读单写操作。
*/


/**
 * @brief 仿照kbuffer实现一个一读一写的高速无锁ringbuffer;
 * @attention 这里的大小是2的整数倍大小，建议使用大小为2的整数倍，避免大量空间浪费
 * @details 
 * <i>参考链接:</i> <a href="https://blog.csdn.net/linyt/article/details/53355355">巧夺天工的kfifo(修订版）</a>
 */
class BaseSingleRingBufferData
{
    typedef uint32_t index_type;
    typedef uint8_t data_type;

public:
    /**
     * @brief 空值构造函数
     */
    BaseSingleRingBufferData();
    /**
     * @brief  构造函数
     * @param  buffer_size      指定缓冲区大小
     */
    BaseSingleRingBufferData(uint32_t buffer_size = 0);
    /**
     * @brief 虚析构函数，方便继承
     */
    virtual ~BaseSingleRingBufferData();
    /**
     * @brief  获取当前buffer的大小
     * @return uint32_t buffer对应大小
     */
    inline uint32_t Size() { return size_; }
    /*  数据指针 */
    inline data_type *Data() { return data_; }
    /* 释放内存 */
    inline void Free()
    {
        if (data_) {
            free(data_);
        }
    }
    /* 重新设置in和out指针，并重置buffer，清楚数据 */
    inline void Clean()
    {
        Reset();
        memset(data_, 0, sizeof(data_type) * size_);
    }
    /* 重新设置指针，不清除数据 */
    inline void Reset()
    {
        in_ptr_ = 0;
        out_ptr_ = 0;
    }
    /* 只要总空间够，就强制进行写入 */
    uint32_t ForceWrite(const uint8_t *new_data, uint32_t len);
    /* 读写函数,当其空间不足时，直接返回失败 */
    uint32_t BaseWrite(const uint8_t *new_data, uint32_t len);
    /* 读取函数 */
    uint32_t BaseRead(uint8_t *read, uint32_t len);
    /* 返回长度，这里会因为总长度是2的倍数而得到正确的值*/
    uint32_t GetDataLen();
    /* 只移动右指针不进行任何操作,返回移动的值 */
    uint32_t MoveOut(const uint32_t len);
    /* 只移动in指针，不做任何操作，返回移动的值 */
    uint32_t MoveIn(const uint32_t len);

private:
    //Ring buffer init
    bool Initialize(uint32_t buffer_size);

    /* 将关键的指针原子化，方便进行操作 */
    index_type in_ptr_;  /* 输入指针 */
    index_type out_ptr_; /* 输出指针 */
    data_type *data_;    /* 缓冲区指针 */
    index_type size_;    /* 缓冲区的总的大小 */
};

/**
 * 使用读写锁定义多线程安全的高效ringbuffer基本data操作元素
 * */

/**
 * 对单个数据进行封装，设置通用模板类ringsinglebuffer;
 * 直接通过类和长度直接进行定义，单环形缓冲区
*/
template <typename T>
class SingleRingBuffer
{
    typedef BaseSingleRingBufferData BaseData;

public:
    /* 空构造函数，一般不建议使用 */
    SingleRingBuffer();
    /* 直接根据长度和类型进行数据的获取 */
    SingleRingBuffer(const uint32_t block_size = 0,
                     const uint32_t length = 0);

    ~SingleRingBuffer();
    /* 每次存取个数据,注意存在exten指针,注意这里buffer内存一定要给够,并且是单存单取 */
    uint32_t pop(T *buffer, uint8_t *extend_ptr = nullptr);
    /* 添加数据，注意格外数据指针 */
    uint32_t push(T *buffer, uint8_t *extend_ptr = nullptr);

private:
    /* 释放缓存内存块 */
    inline void FreeBuffer()
    {
        if (buffer_data_)
        {
            delete buffer_data_;
            buffer_data_ = nullptr;
        }
    };
    BaseData *buffer_data_;          /* 数据存储基本对象；这里的主要数据对象在堆上进行分配，应该减少释放 */
    uint32_t block_data_size_;       /* 存储每个数据的大小 */
    uint32_t buffer_length_;         /* 其中存在的数据长度 */
    bool isExtend_ = false;          /*  是否需要进行扩充，当存在额外的数据时，需要考虑额外的数据扩充 */
    uint32_t block_reall_data_size_; /* 用于记录真正的存储数据的长度，方便变长计算 */
};
/* 指定构造函数 */
template <typename T>
SingleRingBuffer<T>::SingleRingBuffer(const uint32_t block_size, const uint32_t length) : buffer_data_(nullptr),
                                                                                          block_data_size_(block_size),
                                                                                          buffer_length_(length)
{
    /* 计算每个数据块的长度 */
    uint32_t type_size = sizeof(T);
    /* 大于时表明存在数据指针，需要连同数据指针地址一起分配 */
    if (block_size > type_size)
    {
        /* 计算真实的数据地址的偏移 */
        block_reall_data_size_ = block_data_size_ - type_size;
        /* 表明需要额外的数据进行扩充 */
        isExtend_ = true;
    }
    else
    {
        //更新块大小
        block_data_size_ = type_size;
    }
    /* 对buffer进行内存分配 */
    FreeBuffer();
    /* 创建缓冲内存块 */
    buffer_data_ = new BaseData(buffer_length_ * block_data_size_);
}
template <typename T>
SingleRingBuffer<T>::~SingleRingBuffer()
{
    FreeBuffer();
}
template <typename T>
SingleRingBuffer<T>::SingleRingBuffer() : buffer_data_(nullptr), buffer_length_(0), block_data_size_(0), block_reall_data_size_(0)
{
}
template <typename T>
uint32_t SingleRingBuffer<T>::pop(T *buffer, uint8_t *extend_ptr)
{
    /* 计算读取的size */
    uint32_t ret = 0;
    uint32_t read_size = block_data_size_;
    ret = buffer_data_->BaseRead((uint8_t *)buffer, read_size);
    if (isExtend_)
    {
        /* 如果存在扩展;更新扩展指针 */
        if (extend_ptr != nullptr)
        {
            /* 更新指针 */
            extend_ptr = (uint8_t *)buffer + sizeof(buffer);
        }
        else
        {
            std::cout << "Waing!!! Your data need a extend data pointer" << std::endl;
        }
    };
    return ret;
}
/**
 * 双重内存屏障进行写入
 * 
 * 
*/
template <typename T>
uint32_t SingleRingBuffer<T>::push(T *buffer, uint8_t *extend_ptr)
{
    uint32_t ret = 0;
    __sync_synchronize();
    ret = buffer_data_->ForceWrite(buffer, sizeof(buffer));
    ret += buffer_data_->ForceWrite(extend_ptr, block_reall_data_size_);
    __sync_synchronize();
    return ret;
}
NAMESPACE_END

#endif //BASE_RING_BUFFER_H
