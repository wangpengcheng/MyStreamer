#include "base_ring_buffer.h"

NAMESPACE_START


BaseSingleRingBufferData::BaseSingleRingBufferData(uint32_t buffer_size):
		size_(buffer_size),in_ptr_(0),out_ptr_(0),data_(nullptr)
{
    Initialize(size_);
}
BaseSingleRingBufferData::~BaseSingleRingBufferData()
{
    Free();
}
/* 成功直接返回 */
bool BaseSingleRingBufferData::Initialize(uint32_t buffer_size)
{
    /* 检查大小写 */
    if(!is_power_of_2(buffer_size)){
        buffer_size=roundup_power_of_2(buffer_size);
    }
    /* 进行内存分配 */
    data_=(data_type*)(malloc(buffer_size*sizeof(data_type)));
    size_=buffer_size;
	return (data_!=nullptr);
};
/* 返回数据的长度 */
uint32_t BaseSingleRingBufferData::GetDataLen()
{
	/* 获取外部指针注意这里的 */
	register uint32_t out;
	out=out_ptr_;
	return (out_ptr_<in_ptr_?(in_ptr_-out_ptr_):(size_-out_ptr_+in_ptr_));
}
uint32_t BaseSingleRingBufferData::MoveOut(const uint32_t len)
{

	uint32_t move_max_size;
	/* 注意不能超过in_ptr指针 */
	move_max_size=std::min(len,in_ptr_-out_ptr_);
	out_ptr_+=move_max_size;
	return move_max_size;
}
uint32_t BaseSingleRingBufferData::MoveIn(const uint32_t len)
{
	uint32_t move_max_size;
	move_max_size=std::min(len,size_-in_ptr_+out_ptr_);
	in_ptr_+=move_max_size;
	return move_max_size;
}
/* 数据的基础存放 */
uint32_t BaseSingleRingBufferData::BaseRead(uint8_t *read,uint32_t len)
{
	/* 定义左边的指针 */
	uint32_t l;
	/* 计算真实的长度 */
	len=std::min(len,in_ptr_-out_ptr_);
	/*
	这里使用原子操作
	* Ensure that we sample the fifo->in index -before- we
    * start removing bytes from the kfifo.
	*/
 	__sync_synchronize();
    /* first get the data from fifo->out until the end of the buffer */
	/* 注意这里的 out_ptr_ & (size_ - 1)是直接进行模运算*/
    l = std::min(len, size_ - (out_ptr_ & (size_ - 1)));
	/* 拷贝右半部分 */
    memcpy(read, data_ + (out_ptr_ & (size_ - 1)), l);
    /* then get the rest (if any) from the beginning of the buffer */
	/* 拷贝左半部分 */
    memcpy(read + l, data_, len - l);
    /*
    * Ensure that we remove the bytes from the kfifo -before-
    * * we update the fifo->out index.
    * */
    __sync_synchronize();
    out_ptr_ += len;
    return len;
}
uint32_t BaseSingleRingBufferData::BaseWrite(const uint8_t *new_data,uint32_t len)
{
	uint32_t l;
	len=std::min(len,size_-in_ptr_+out_ptr_);
	__sync_synchronize();
	/* first put the data starting from fifo->in to buffer end */
	l = std::min(len, size_ - (in_ptr_  & (size_ - 1)));
	memcpy(data_+(in_ptr_&(size_-1)),new_data,l);
	/* then put the rest (if any) at the beginning of the buffer */
	memcpy(data_,new_data+l,len-l);
	  /*

    * Ensure that we add the bytes to the kfifo -before-

    * we update the fifo->in index.

    */
	__sync_synchronize();
	in_ptr_+=len;
	return len;

}
uint32_t BaseSingleRingBufferData::ForceWrite(const uint8_t *new_data,uint32_t len)
{
	if(len>size_){
        return 0;
    }
	/* 可移动的距离 */
	uint32_t l;
	/* 查找左边界 */
	l = std::min(len, size_ - in_ptr_ + out_ptr_);
	uint32_t ret=0;
	if(!l){
		if((ret=MoveOut(len))){
			return 0;
		}
	}
	return BaseWrite(new_data,len);
}

NAMESPACE_END