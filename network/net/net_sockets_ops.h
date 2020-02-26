/**
 * 对socket的相关函数，进行操作
*/

#ifndef NET_SOCKETSOPS_H
#define NET_SOCKETSOPS_H
#include <arpa/inet.h>
#include "base_tool.h"

NAMESPACE_START

namespace net
{
namespace sockets
{
/* 创建一个非阻塞的 socket file 描述符*/
int createNonblockingOrDie(sa_family_t family);
/* 连接文件描述符 */
int  connect(int sockfd, const struct sockaddr* addr);
/* 连接socket和端口 */
void bindOrDie(int sockfd, const struct sockaddr* addr);
/* 设置被动监听 */
void listenOrDie(int sockfd);
/* 捕获事件 */
int  accept(int sockfd, struct sockaddr_in6* addr);
/* read函数 */
ssize_t read(int sockfd, void *buf, size_t count);
/*  */
ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
/* 写函数 */
ssize_t write(int sockfd, const void *buf, size_t count);
/* 关闭连接符 */
void close(int sockfd);
/* 关闭连接符 */
void shutdownWrite(int sockfd);
/*  */
void toIpPort(char* buf, size_t size,
              const struct sockaddr* addr);
/*  */
void toIp(char* buf, size_t size,
          const struct sockaddr* addr);

void fromIpPort(const char* ip, uint16_t port,
                struct sockaddr_in* addr);
void fromIpPort(const char* ip, uint16_t port,
                struct sockaddr_in6* addr);

/* socket错误信息 */
int getSocketError(int sockfd);
/* 相关结构体和数据结构的转换 */
const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);
/* 静态转换函数 */
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);
const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr);

struct sockaddr_in6 getLocalAddr(int sockfd);
struct sockaddr_in6 getPeerAddr(int sockfd);
/* 是否为服务器主连接函数 */
bool isSelfConnect(int sockfd);
}
}

NAMESPACE_END
#endif