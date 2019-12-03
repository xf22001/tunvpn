

/*================================================================
 *
 *
 *   文件名称：linux_tun.cpp
 *   创 建 者：肖飞
 *   创建日期：2019年11月28日 星期四 11时21分13秒
 *   修改日期：2019年12月03日 星期二 12时55分50秒
 *   描    述：
 *
 *================================================================*/
#include "linux_tun.h"

#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "util_log.h"

linux_tun::linux_tun()
{
	util_log *l = util_log::get_instance();

	l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
	tap_fd = -1;
}

linux_tun::~linux_tun()
{
	util_log *l = util_log::get_instance();

	l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
}

int linux_tun::tun_ioctl(int fd, int request)
{
	int ret;
	//util_log *l = util_log::get_instance();
	int size = tap_name.size();

	if(size >= IFNAMSIZ) {
		size = IFNAMSIZ - 1;
	}

	if(fd < 0) {
		ret = -1;
		return ret;
	}

	strncpy(ifr.ifr_name, tap_name.c_str(), size);
	ifr.ifr_name[size] = 0;

	ret = ioctl(fd, request, (void *)&ifr);

	return ret;
}

int linux_tun::update_tun_info()
{
	int ret = -1;
	util_log *l = util_log::get_instance();
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in *sin;
	char buffer[32];

	if(sockfd < 0) {
		return ret;
	}

	//mac addr
	ret = tun_ioctl(sockfd, SIOCGIFHWADDR);

	if(ret < 0) {
		l->printf("ioctl %s failed!(%s)\n", "SIOCGIFHWADDR", strerror(errno));
		goto fail;
	}

	l->printf("mac:%02x:%02x:%02x:%02x:%02x:%02x\n",
	          (unsigned char)ifr.ifr_hwaddr.sa_data[0],
	          (unsigned char)ifr.ifr_hwaddr.sa_data[1],
	          (unsigned char)ifr.ifr_hwaddr.sa_data[2],
	          (unsigned char)ifr.ifr_hwaddr.sa_data[3],
	          (unsigned char)ifr.ifr_hwaddr.sa_data[4],
	          (unsigned char)ifr.ifr_hwaddr.sa_data[5]);

	memcpy(tun_info.mac_addr, ifr.ifr_hwaddr.sa_data, IFHWADDRLEN);

	//ip addr
	sin = (struct sockaddr_in *)&tun_info.ip;
	ret = tun_ioctl(sockfd, SIOCGIFADDR);

	if(ret < 0) {
		l->printf("ioctl %s failed!(%s)\n", "SIOCGIFADDR", strerror(errno));
		goto fail;
	}

	memcpy(sin, &ifr.ifr_addr, sizeof(struct sockaddr_in));
	//l->dump((const char *)sin, sizeof(struct sockaddr_in));
	inet_ntop(AF_INET, &sin->sin_addr, buffer, sizeof(buffer));

	l->printf("ip addr:%s\n", buffer);

	//ip mask
	sin = (struct sockaddr_in *)&tun_info.netmask;
	ret = tun_ioctl(sockfd, SIOCGIFNETMASK);

	if(ret < 0) {
		l->printf("ioctl %s failed!(%s)\n", "SIOCGIFNETMASK", strerror(errno));
		goto fail;
	}

	memcpy(sin, &ifr.ifr_netmask, sizeof(struct sockaddr_in));
	//l->dump((const char *)sin, sizeof(struct sockaddr_in));
	inet_ntop(AF_INET, &sin->sin_addr, buffer, sizeof(buffer));

	l->printf("netmask:%s\n", buffer);


	ifr.ifr_mtu = 1400;
	ret = tun_ioctl(sockfd, SIOCSIFMTU);
	if(ret < 0) {
		l->printf("ioctl %s failed!(%s)\n", "SIOCSIFMTU", strerror(errno));
		goto fail;
	}

	ret = 0;

	return ret;
fail:

	if(sockfd > 0) {
		close(sockfd);
	}

	return ret;
}

int linux_tun::open_tun(std::string request_name)
{
	util_log *l = util_log::get_instance();
	std::string tun_device = "/dev/net/tun";
	int ret = -1;
	//int flags;

	l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);

	tap_name = request_name;

	tap_fd = open(tun_device.c_str(), (O_RDWR | O_NONBLOCK));

	if(tap_fd < 0) {
		l->printf("open %s failed!(%s)\n", tun_device.c_str(), strerror(errno));
		goto failed;
	}

	ifr.ifr_flags = (IFF_TAP | IFF_NO_PI);
	ret = tun_ioctl(tap_fd, TUNSETIFF);

	if(ret < 0) {
		l->printf("ioctl %s failed!(%s)\n", "TUNSETIFF", strerror(errno));
		goto failed;
	}

	tap_name = ifr.ifr_name;

	//flags = fcntl(tap_fd, F_GETFL, 0);
	//flags |= O_NONBLOCK;
	//flags = fcntl(tap_fd, F_SETFL, flags);


	ret = 0;

	return ret;
failed:

	if(tap_fd >= 0) {
		close(tap_fd);
		tap_fd = -1;
	}

	return ret;
}

std::string linux_tun::get_tap_name()
{
	return tap_name;
}

int linux_tun::get_tap_fd()
{
	return tap_fd;
}

tun_info_t *linux_tun::get_tun_info()
{
	return &tun_info;
}
