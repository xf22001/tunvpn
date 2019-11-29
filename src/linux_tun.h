

/*================================================================
 *
 *
 *   文件名称：linux_tun.h
 *   创 建 者：肖飞
 *   创建日期：2019年11月28日 星期四 12时30分42秒
 *   修改日期：2019年11月29日 星期五 11时45分10秒
 *   描    述：
 *
 *================================================================*/
#ifndef _LINUX_TUN_H
#define _LINUX_TUN_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <net/if.h>
#include <linux/if_tun.h>

#include <netdb.h>
#include <netinet/in.h>
//#include <sys/select.h>
#include <sys/socket.h>

#ifdef __cplusplus
}
#endif

#include <string>

class linux_tun
{
private:
	struct ifreq ifr;
	std::string tap_name;
	int tap_fd;
	unsigned char mac_addr[IFHWADDRLEN];
	struct sockaddr ip_addr;
	struct sockaddr netmask;
protected:
public:
	linux_tun();
	virtual ~linux_tun();

	int tun_ioctl(int fd, int request);
	int open_tun(std::string request_name);
	int get_tun_info();
	std::string get_tap_name();
	int get_tap_fd();
	unsigned char *get_tap_mac();
	struct sockaddr *get_ip_addr();
	struct sockaddr *get_netmask();
};

#endif //_LINUX_TUN_H
