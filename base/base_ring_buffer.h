/**
 * 定义ringbuffer方便使用，basebuffer使用模板进行初始化。
 * 定义通用一般接口，并对char进行特例化，增加write和read函数。
 * buffer为多线程安全
 */

#ifndef BASE_RING_BUFFER_H
#define BASE_RING_BUFFER_H

#include <vector>
#include <memory>
#include <atomic>
#include <iostream>

#include <stdint.h>

#include "base_tool.h"

NAMESPACE_START

/**
 * 简单环形缓冲区模板
 * https://blog.csdn.net/weixin_40825228/article/details/80783860?depth_1.utm_source=distribute.pc_relevant.none-task&utm_source=distribute.pc_relevant.none-task
 */
#define DEFAULT_SIZE 30
template<typename T>
class CircuBuffer
{
public:
	CircuBuffer(unsigned size):
		size_(size)
	{
		writeIndex_=0;
		readIndex_=0;
		buffer_=new T[size_];
	}
 
	CircuBuffer():
		size_(DEFAULT_SIZE)
	{
		writeIndex_=0;
		readIndex_=0;
		buffer_=new T[size_];
	}
 
	~CircuBuffer()
	{
		delete [] buffer_;
	}
 
	unsigned getReadIndex() const
	{
		return readIndex_;
	}
 
	unsigned getWriteIndex() const
	{
		return writeIndex_;
	}
 
	bool isEmpty()
	{
		return writeIndex_==readIndex_;
	}
 
	bool isFull()
	{
		return (readIndex_+1)%size_==writeIndex_;
	}
 
	bool pushAnEle(T element)
	{
		//队列不为满
		if(!isFull())
		{
			buffer_[writeIndex_]=element;
			writeIndex_=(writeIndex_+1)%size_;
			return true;
		}
		else
		{
			return false;
		}
 
	};
 
	T* getAnEle()
	{
		//队列不为空
		if (!isEmpty())
		{
			T* temp=buffer_+readIndex_;
			readIndex_=(readIndex_+1)%size_;
			return temp;
		}
		else
		{
			return nullptr;
		}
	}
private:
 
	//写指针
	unsigned writeIndex_;
	//读指针
	unsigned readIndex_;
	//环形队列首地址
	T* buffer_;
	//环形队列的尺寸
	unsigned size_;
};

/**
 * 这里仿照kbuffer实现一个一读一写的高速无锁ringbuffer;
 * 注意这里的大小是2的整数倍大小，建议使用大小为2的整数倍，避免大量空间浪费
 * */

/*判断n是否为2的幂*/
static bool is_power_of_2(unsigned int n)
{
    return (n != 0 && ((n & (n - 1)) == 0));
}
 
/*将数字a向上取整为2的次幂*/
static uint32_t roundup_power_of_2(uint32_t a)
{
    if (a == 0)
        return 0;
 
    uint32_t position = 0;
    for (int i = a; i != 0; i >>= 1)
        position++;
 
    return (uint32_t)(1 << position);
};


/**
 * 参考kfifo的basebufferdata基础数据管理类，
 * 注意这里没有加锁，因此是属于单读单写操作。
*/
class BaseRingBufferData
{
public:
    BaseRingBufferData();
    BaseRingBufferData(uint32_t buffer_size);
    ~BaseRingBufferData();
    /* 读写函数,当其空间不足时，直接返回失败 */
    bool BaseWrite(const uint8_t *new_data,uint32_t len);
    bool BaseRead(uint8_t *read,uint32_t len);
private:
    //Ring buffer init
    void Init(uint32_t buffer_size);
    //free data
    void Free();
    //reset
    void Reset();
    /* 将关键的指针原子化，方便进行操作 */
    std::atomic<uint32_t>    in_ptr_;                    /* 输入指针 */
    std::atomic<uint32_t>    out_ptr_;                   /* 输出指针 */
    uint8_t*    data_;                                   /* 缓冲区指针 */
    std::atomic<uint32_t>    size_;                      /* 缓冲区的总的大小 */
};
BaseRingBufferData::BaseRingBufferData(uint32_t buffer_size)
{
    Init(buffer_size);
}
BaseRingBufferData::~BaseRingBufferData()
{
    Free();
}
void BaseRingBufferData::Reset()
{
    in_ptr_=out_ptr_=0;
}
void BaseRingBufferData::Init(uint32_t buffer_size)
{
    /* 检查大小写 */
    if(!is_power_of_2(buffer_size)){
        buffer_size=roundup_power_of_2(buffer_size);
    }
    //进行内存分配
    data_=(uint8_t*)(malloc(buffer_size*sizeof(uint8_t)));
    in_ptr_=out_ptr_=0;
    size_=buffer_size;
}
bool Write
// template <typename T>
// class SingleThreadRingBuffer
// {
//     typedef T value_type;
// public:
//     /* 构造函数；使用T来判断基本类型，并定义最终的长度*/
//     SingleThreadRingBuffer(uint32_t buffer_length);
//     ~SingleThreadRingBuffer();
//     void BufferReset();
//     uint32_t GetLength();
//     /* 重新清除buffer */
//     bool Clear();
//     /* buffer读写函数;是否读取成功 */
//     bool Read(T* r_data,uint32_t read_length=0);
//     /* 返回写入是否成功 */
//     bool Write(T* w_data,uint32_t write_length=0);
//     bool WriteForce(T* w_data,uint32_t write_length=0);
// private:
//     /* 初始化buffer */
//     bool BufferInit(uint32_t buffer_length);
//     void BufferFree();
// private:
//     uint32_t block_size(0);                            /* 关键单元块的大小 */
//     std::atomic_uint_fast32_t   buffer_size(0);            /* 缓冲区的长度，大小为2的指数倍数;注意这里是指T的个数的长度 */
//     std::atomic_uint_fast32_t   data_length(0);                  /* 设置队列中存储的数据长度 */
//     std::atomic_uint_fast32_t   in_index(0);               /* 输入指针 */
//     std::atomic_uint_fast32_t   out_index(0);              /* 输入指针 */
//     uint8_t  *data=nullptr;                                        /* 存储数据指针 */ 
// }; 
// /*=====  SingleThreadRingBuffer  =====*/
// template <typename T>
// SingleThreadRingBuffer<T>::SingleThreadRingBuffer(uint32_t buffer_length)
// {
//     BufferInit(buffer_length);

// }

// template <typename T>
// SingleThreadRingBuffer<T>::~SingleThreadRingBuffer()
// {
//     BufferFree();
// }
// template <typename T>
// void SingleThreadRingBuffer<T>::BufferFree()
// {
//     if(data){
//         free(data);
//         data=nullptr;
//     }
// }
// template <typename T>
// void SingleThreadRingBuffer<T>::BufferReset()
// {
//     //将所有的索引重现设置，并释放内存;注意size大小不变
//     in_index=0;
//     out_index=0;
//     data_length=0;

// }
// template <typename T>
// bool SingleThreadRingBuffer<T>::BufferInit(uint32_t buffer_length)
// {
//     /* 计算基础的块的大小 */
//     block_size=sizeof(T);
//     /* 进行内存分配，注意大内存分配错误 */
//     data=(uint8_t*)malloc(block_size*buffer_length);
//     buffer_size=buffer_length;
// }

// template <typename T>
// uint32_t SingleThreadRingBuffer<T>::GetLength()
// {
//     return data_length;
// }

/*=====  SingleThreadRingBuffer  =====*/


NAMESPACE_END

#endif //BASE_RING_BUFFER_H
