

/*================================================================
 *
 *
 *   文件名称：linux_tun.cpp
 *   创 建 者：肖飞
 *   创建日期：2019年11月28日 星期四 11时21分13秒
 *   修改日期：2019年11月28日 星期四 15时33分56秒
 *   描    述：
 *
 *================================================================*/
#include "linux_tun.h"

#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "util_log.h"

linux_tun::linux_tun()
{
	util_log *l = util_log::get_instance();

	l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
	tap_fd = -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	ifr.ifr_flags = (IFF_TAP | IFF_NO_PI);
}

linux_tun::~linux_tun()
{
	util_log *l = util_log::get_instance();

	l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
}

int linux_tun::open_tun(std::string request_name)
{
	util_log *l = util_log::get_instance();
	std::string tun_device = "/dev/net/tun";
	int size = request_name.size();
	int ret = -1;

	l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);

	if(size >= IFNAMSIZ) {
		size = IFNAMSIZ - 1;
	}

	strncpy(ifr.ifr_name, request_name.c_str(), size);
	ifr.ifr_name[size] = 0;

	l->printf("ifr.ifr_name:%s\n", ifr.ifr_name);
	l->dump((const char *)ifr.ifr_name, IFNAMSIZ);

	tap_fd = open(tun_device.c_str(), (O_RDWR | O_NONBLOCK));

	if(tap_fd < 0) {
		l->printf("open %s failed!(%s)\n", tun_device.c_str(), strerror(errno));
		goto failed;
	}

	ret = ioctl(tap_fd, TUNSETIFF, (void *)&ifr);

	if(ret < 0) {
		l->printf("ioctl %s failed!(%s)\n", "TUNSETIFF", strerror(errno));
		goto failed;
	}

	tap_name = ifr.ifr_name;

	ret = 0;

	return ret;
failed:

	if(tap_fd > 0) {
		close(tap_fd);
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
