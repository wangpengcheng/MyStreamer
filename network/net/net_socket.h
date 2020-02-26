/**
 * socket基础连接操作类
 * 
*/

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include "uncopyable.h"

struct tcp_info;

NAMESPACE_START

namespace net
{

class InetAddress;

/**
 * socket描述符的基本操作；保证线程安全
*/
class Socket : noncopyable
{
 public:
  explicit Socket(int sockfd)
    : sockfd_(sockfd)
  { }

  /* 析构函数 */
    ~Socket();
    /* socket文件描述符 */
    int fd() const { return sockfd_; }
  /* 返回TCP信息 */
  bool getTcpInfo(struct tcp_info*) const;
  bool getTcpInfoString(char* buf, int len) const;

  /* 检查地址是否正在被使用 */
  void bindAddress(const InetAddress& localaddr);
  /* 进入被动监听状态 */
    void listen();

 
  /* 进行监听状态，返回生成的socket文件描述符，错误返回-1 */
  int accept(InetAddress* peeraddr);
    /* 关闭写入 */
  void shutdownWrite();

  /**
   * 禁用TCP_NODELAY，减少缓冲区，允许小包的发送
   * https://blog.csdn.net/lclwjl/article/details/80154565
   *  */
  void setTcpNoDelay(bool on);

  /**
   * 是否允许端口释放后立即再次使用
   * https://www.cnblogs.com/HKUI/p/11707170.html
  */
  void setReuseAddr(bool on);

  /**
   * 是否允许端口复用
  */
  void setReusePort(bool on);

  /**
   * 是否使用keepalive检测，等待一定时间的TCP传输连接
   * https://www.cnblogs.com/tekkaman/p/4849767.html
  */
  void setKeepAlive(bool on);

 private:
  const int sockfd_;            /* socket连接文件描述符 */
};

}  // namespace net

NAMESPACE_END



#endif