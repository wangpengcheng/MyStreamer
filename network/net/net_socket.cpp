#include "logging.h"
#include "net_socket.h"
#include "net_inet_address.h"
#include "net_sockets_ops.h"
#include "base_define.h"
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h> // snprintf

using namespace MY_NAME_SPACE;
using namespace MY_NAME_SPACE::net;

Socket::~Socket()
{
    sockets::close(sockfd_);
}
/* 获取TCP相关信息 */
bool Socket::getTcpInfo(struct tcp_info *tcpi) const
{
    socklen_t len = sizeof(*tcpi);
    memZero(tcpi, len);
    return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
}
/* 将TCP信息转换为string */
bool Socket::getTcpInfoString(char *buf, int len) const
{
    struct tcp_info tcpi;
    bool ok = getTcpInfo(&tcpi);
    if (ok)
    {
        snprintf(buf, len, "unrecovered=%u "
                           "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
                           "lost=%u retrans=%u rtt=%u rttvar=%u "
                           "sshthresh=%u cwnd=%u total_retrans=%u",
                 tcpi.tcpi_retransmits, // Number of unrecovered [RTO] timeouts
                 tcpi.tcpi_rto,         // Retransmit timeout in usec
                 tcpi.tcpi_ato,         // Predicted tick of soft clock in usec
                 tcpi.tcpi_snd_mss,
                 tcpi.tcpi_rcv_mss,
                 tcpi.tcpi_lost,    // Lost packets
                 tcpi.tcpi_retrans, // Retransmitted packets out
                 tcpi.tcpi_rtt,     // Smoothed round trip time in usec
                 tcpi.tcpi_rttvar,  // Medium deviation
                 tcpi.tcpi_snd_ssthresh,
                 tcpi.tcpi_snd_cwnd,
                 tcpi.tcpi_total_retrans); // Total retransmits for entire connection
    }
    return ok;
}

void Socket::bindAddress(const InetAddress &addr)
{
    sockets::bindOrDie(sockfd_, addr.getSockAddr());
}

void Socket::listen()
{
    sockets::listenOrDie(sockfd_);
}
/* accpet返回新的文件连接符 */
int Socket::accept(InetAddress *peeraddr)
{
    struct sockaddr_in6 addr;
    memZero(&addr, sizeof addr);
    int connect_fd = sockets::accept(sockfd_, &addr);
    if (connect_fd >= 0)
    {
        peeraddr->setSockAddrInet6(addr);
    }
    return connect_fd;
}

void Socket::shutdownWrite()
{
    sockets::shutdownWrite(sockfd_);
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                 &optval, static_cast<socklen_t>(sizeof optval));
    // FIXME CHECK
}

void Socket::setReusePort(bool on)
{
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                           &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on)
    {
        LOG_SYSERR << "SO_REUSEPORT failed.";
    }
#else
    if (on)
    {
        LOG_ERROR << "SO_REUSEPORT is not supported.";
    }
#endif
}
/* 选择保持连接 */
void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
                 &optval, static_cast<socklen_t>(sizeof(optval)));
    // FIXME CHECK
}