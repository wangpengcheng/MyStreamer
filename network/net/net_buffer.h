#ifndef NET_BUFFER_H
#define NET_BUFFER_H

#include "uncopyable.h"
#include "string_piece.h"
#include "base_types.h"
#include "net_endian.h"

#include <algorithm>
#include <vector>
#include <assert.h>
#include <string.h>

//#include <unistd.h>  // ssize_t
NAMESPACE_START

namespace net
{

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

/*
 *        
 *   缓冲区的设计方法，muduo采用vector连续内存作为缓冲区，libevent则是分块内存
 *      1.相比之下，采用vector连续内存更容易管理，同时利用std::vector自带的内存
 *        增长方式，可以减少扩充的次数（capacity和size一般不同）
 *      2.记录缓冲区数据起始位置和结束位置，写入时写到已有数据的后面，读出时从
 *        数据起始位置读出
 *      3.起始/结束位置如上图的readerIndex/writeIndex，其中readerIndex为缓冲区
 *        数据的起始索引下标，writeIndex为结束位置下标。采用下标而不是迭代器的
 *        原因是删除(erase)数据时迭代器可能失效
 *      4.开头部分(readerIndex以前)是预留空间，通常只有几个字节的大小，可以用来
 *        写入数据的长度，解决粘包问题
 *      5.读出和写入数据时会动态调整readerIndex/writeIndex，如果没有数据，二者
 *        相等
 */
class Buffer : public copyable
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize),
        readerIndex_(kCheapPrepend),
        writerIndex_(kCheapPrepend)
    {
        assert(readableBytes() == 0);
        assert(writableBytes() == initialSize);
        assert(prependableBytes() == kCheapPrepend);
    }

    // implicit copy-ctor, move-ctor, dtor and assignment are fine
    // NOTE: implicit move-ctor is added in g++ 4.6

    void swap(Buffer& rhs)
    {
        buffer_.swap(rhs.buffer_);
        std::swap(readerIndex_, rhs.readerIndex_);
        std::swap(writerIndex_, rhs.writerIndex_);
    }
    /* 可读的数据就是起始位置和结束位置中间的部分 */
    size_t readableBytes() const
    { return writerIndex_ - readerIndex_; }

    size_t writableBytes() const
    { return buffer_.size() - writerIndex_; }

    size_t prependableBytes() const
    { return readerIndex_; }
    /* 返回数据起始位置 */
    const char* peek() const
    { return begin() + readerIndex_; }

    const char* findCRLF() const
    {
        // FIXME: replace with memmem()?
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    const char* findCRLF(const char* start) const
    {
        assert(peek() <= start);
        assert(start <= beginWrite());
        // FIXME: replace with memmem()?
        const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF+2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    const char* findEOL() const
    {
        const void* eol = memchr(peek(), '\n', readableBytes());
        return static_cast<const char*>(eol);
    }

    const char* findEOL(const char* start) const
    {
        assert(peek() <= start);
        assert(start <= beginWrite());
        const void* eol = memchr(start, '\n', beginWrite() - start);
        return static_cast<const char*>(eol);
    }

    /* 调整readerIndex，后移len */
    void retrieve(size_t len)
    {
        assert(len <= readableBytes());
        /* 
        * 如果调整后仍然有数据，就将readerIndex增加len
        * 如果已经将数据全部读完(len >= readableBytes)，那么就初始化readerIndex/writerIndex位置
        */
        if (len < readableBytes())
        {
        readerIndex_ += len;
        }
        else
        {
            /* 如果数据全部被用户读出，就重新调整readerIndex/writerIndex位置 */
            retrieveAll();
        }
    }

    void retrieveUntil(const char* end)
    {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }

    void retrieveInt64()
    {
        retrieve(sizeof(int64_t));
    }

    void retrieveInt32()
    {
        retrieve(sizeof(int32_t));
    }

    void retrieveInt16()
    {
        retrieve(sizeof(int16_t));
    }

    void retrieveInt8()
    {
        retrieve(sizeof(int8_t));
    }
    /* 初始化readerIndex/writerIndex位置，通常在用户将数据全部读出之后执行 */
    void retrieveAll()
    {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }
    /* 从缓冲区中读取所有数据 */
    string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }
    /* 从缓冲区中读取len个字节的数据 */
    string retrieveAsString(size_t len)
    {
        assert(len <= readableBytes());
        /* peek返回数据的起点 */
        /* 调用string(const char* s, size_type n);构造函数，初始化为从地址s开始的n个字节 */
        /* 调整缓冲区，即改变readerIndex的位置，后移len */
        string result(peek(), len);
        retrieve(len);
        return result;
    }

    StringPiece toStringPiece() const
    {
        return StringPiece(peek(), static_cast<int>(readableBytes()));
    }

    void append(const StringPiece& str)
    {
        append(str.data(), str.size());
    }
    /**
     * 如果读缓冲区大小不够，其他数据就会写入到栈空间，
     * 接下来需要将栈空间的数据追加到缓冲区的末尾，使用append函数
    */
    void append(const char* /*restrict*/ data, size_t len)
    {
        /* 确保有足够的空间容纳len大小的数据 */
        ensureWritableBytes(len);
        /* 将数据copy到writerIndex后面，beginWrite返回的就是writerIndex位置的地址（writerIndex是下标） */
        /* 写完数据，更新writerIndex */
        std::copy(data, data+len, beginWrite());
        hasWritten(len);
    }

    void append(const void* /*restrict*/ data, size_t len)
    {
        append(static_cast<const char*>(data), len);
    }
    /* 返回剩余可用空间大小，如果不足len，开辟新空间（调用resize） */
    void ensureWritableBytes(size_t len)
    {
        if (writableBytes() < len)
        {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    char* beginWrite()
    { return begin() + writerIndex_; }

    const char* beginWrite() const
    { return begin() + writerIndex_; }

    void hasWritten(size_t len)
    {
        assert(len <= writableBytes());
        writerIndex_ += len;
    }

    void unwrite(size_t len)
    {
        assert(len <= readableBytes());
        writerIndex_ -= len;
    }

    ///
    /// Append int64_t using network endian
    ///
    void appendInt64(int64_t x)
    {
        int64_t be64 = sockets::hostToNetwork64(x);
        append(&be64, sizeof be64);
    }

    ///
    /// Append int32_t using network endian
    ///
    void appendInt32(int32_t x)
    {
        int32_t be32 = sockets::hostToNetwork32(x);
        append(&be32, sizeof be32);
    }

    void appendInt16(int16_t x)
    {
        int16_t be16 = sockets::hostToNetwork16(x);
        append(&be16, sizeof be16);
    }

    void appendInt8(int8_t x)
    {
        append(&x, sizeof x);
    }

    ///
    /// Read int64_t from network endian
    ///
    /// Require: buf->readableBytes() >= sizeof(int32_t)
    int64_t readInt64()
    {
        int64_t result = peekInt64();
        retrieveInt64();
        return result;
    }

    ///
    /// Read int32_t from network endian
    ///
    /// Require: buf->readableBytes() >= sizeof(int32_t)
    int32_t readInt32()
    {
        int32_t result = peekInt32();
        retrieveInt32();
        return result;
    }

    int16_t readInt16()
    {
        int16_t result = peekInt16();
        retrieveInt16();
        return result;
    }

    int8_t readInt8()
    {
        int8_t result = peekInt8();
        retrieveInt8();
        return result;
    }

    ///
    /// Peek int64_t from network endian
    ///
    /// Require: buf->readableBytes() >= sizeof(int64_t)
    int64_t peekInt64() const
    {
        assert(readableBytes() >= sizeof(int64_t));
        int64_t be64 = 0;
        ::memcpy(&be64, peek(), sizeof be64);
        return sockets::networkToHost64(be64);
    }

    ///
    /// Peek int32_t from network endian
    ///
    /// Require: buf->readableBytes() >= sizeof(int32_t)
    int32_t peekInt32() const
    {
        assert(readableBytes() >= sizeof(int32_t));
        int32_t be32 = 0;
        ::memcpy(&be32, peek(), sizeof be32);
        return sockets::networkToHost32(be32);
    }

    int16_t peekInt16() const
    {
        assert(readableBytes() >= sizeof(int16_t));
        int16_t be16 = 0;
        ::memcpy(&be16, peek(), sizeof be16);
        return sockets::networkToHost16(be16);
    }

    int8_t peekInt8() const
    {
        assert(readableBytes() >= sizeof(int8_t));
        int8_t x = *peek();
        return x;
    }

    ///
    /// Prepend int64_t using network endian
    ///
    void prependInt64(int64_t x)
    {
        int64_t be64 = sockets::hostToNetwork64(x);
        prepend(&be64, sizeof be64);
    }

    ///
    /// Prepend int32_t using network endian
    ///
    void prependInt32(int32_t x)
    {
        int32_t be32 = sockets::hostToNetwork32(x);
        prepend(&be32, sizeof be32);
    }

    void prependInt16(int16_t x)
    {
        int16_t be16 = sockets::hostToNetwork16(x);
        prepend(&be16, sizeof be16);
    }

    void prependInt8(int8_t x)
    {
        prepend(&x, sizeof x);
    }
    /* 读取指定长度 */
    void prepend(const void* /*restrict*/ data, size_t len)
    {
        assert(len <= prependableBytes());
        readerIndex_ -= len;
        const char* d = static_cast<const char*>(data);
        std::copy(d, d+len, begin()+readerIndex_);
    }

    void shrink(size_t reserve)
    {
        // FIXME: use vector::shrink_to_fit() in C++ 11 if possible.
        Buffer other;
        other.ensureWritableBytes(readableBytes()+reserve);
        other.append(toStringPiece());
        swap(other);
    }

    size_t internalCapacity() const
    {
        return buffer_.capacity();
    }

    /// Read data directly into buffer.
    ///
    /// It may implement with readv(2)
    /// @return result of read(2), @c errno is saved
    /* 从套接字（内核tcp缓冲区）中读取数据放到读缓冲区中 */
    ssize_t readFd(int fd, int* savedErrno);

    private:

    char* begin()
    { return &*buffer_.begin(); }

    const char* begin() const
    { return &*buffer_.begin(); }
    /* 调整空间大小 */
    void makeSpace(size_t len)
    {
        /* 
        * 在多次从缓冲区读数据后，readerIndex会后移很多，导致预留空间变大
        * 在增大空间之前，先判断调整预留空间的大小后能否容纳要求的数据
        * 如果可以，则将预留空间缩小为8字节（默认的预留空间大小）
        * 如果不可以，那么就只能增加空间
        */
        if (writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            // FIXME: move readable data
            /* writerIndex代表当前缓冲区已使用的大小，调整只需调整到恰好满足len大小即可 */
            buffer_.resize(writerIndex_+len);
        }
        else
        {
            // move readable data to the front, make space inside buffer
            /* 通过缩小预留空间大小可以容纳len个数据，就缩小预留空间 */
            assert(kCheapPrepend < readerIndex_);
            /* 返回缓冲区数据个数，writerIndex - readerIndex */
            size_t readable = readableBytes();
            /* 将所有数据前移 */
            std::copy(begin()+readerIndex_,
                        begin()+writerIndex_,
                        begin()+kCheapPrepend);
            /* 更新两个指针（下标） */
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
            assert(readable == readableBytes());
        }
    }

private:
    std::vector<char> buffer_;              /* 缓冲区 */
    size_t readerIndex_;                    /* 数据起始点 */
    size_t writerIndex_;                    /* 数据结束点 */

    static const char kCRLF[];
};

}  // namespace net

NAMESPACE_END

#endif