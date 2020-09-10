/**
 * 域名地址解析函数
 * 
*/

#ifndef NET_INTE_ADDRESS_H
#define NET_INTE_ADDRESS_H
#include "base_tool.h"
#include "string_piece.h"
#include "uncopyable.h"
/* 以太网控制函数 */
#include <netinet/in.h>
/* 网络地址抽象解析函数 */
NAMESPACE_START
/* net命名空间 */
namespace net
{
    /* net命名空间 */
    namespace sockets
    {
        /* 地址类的静态转换函数 */
        const struct sockaddr *sockaddr_cast(const struct sockaddr_in6 *addr);
    } // namespace sockets

    /**
 *  sockaddr_in的包装类
*/
    class InetAddress : public MyStreamer::copyable
    {

    public:
        /* 构造函数，返回端口号的尾部节点 */
        explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);
        /*** 通过地址和端口进行解析 */
        InetAddress(StringArg ip, uint16_t port, bool ipv6 = false);
        /* 直接通过addr进行构造 */
        explicit InetAddress(const struct sockaddr_in &addr)
            : addr_(addr)
        {
        }

        explicit InetAddress(const struct sockaddr_in6 &addr)
            : addr6_(addr)
        {
        }
        sa_family_t family() const { return addr_.sin_family; }
        std::string toIp() const;
        std::string toIpPort() const;
        uint16_t toPort() const;
        // 默认拷贝转发函数

        const struct sockaddr *getSockAddr() const { return sockets::sockaddr_cast(&addr6_); }
        /* 设置ipv6参数 */
        void setSockAddrInet6(const struct sockaddr_in6 &addr6) { addr6_ = addr6; }
        /* 对应的二进制网络号 */
        uint32_t ipNetEndian() const;
        /* 二进制网络端口号 */
        uint16_t portNetEndian() const { return addr_.sin_port; }

        /*** 解析网络地址到字符串*/
        static bool resolve(StringArg hostname, InetAddress *result);
        // 设置ipv6的 ScopeID--硬件设备号
        void setScopeId(uint32_t scope_id);

    private:
    
        union
        {
            struct sockaddr_in addr_;
            struct sockaddr_in6 addr6_;
        };
    };
} // namespace  net
NAMESPACE_END

#endif