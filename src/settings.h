

/*================================================================
 *
 *
 *   文件名称：settings.h
 *   创 建 者：肖飞
 *   创建日期：2019年11月28日 星期四 17时02分11秒
 *   修改日期：2019年12月01日 星期日 16时14分32秒
 *   描    述：
 *
 *================================================================*/
#ifndef _SETTINGS_H
#define _SETTINGS_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>

#ifdef __cplusplus
}
#endif

#include "configure/configure.h"
#include "linux_tun.h"
#include "tun_socket_notifier.h"

struct sockaddr_less_then {
	bool operator() (const struct sockaddr &addr1, const struct sockaddr &addr2) const
	{
		bool ret = false;
		struct sockaddr_in *in1 = (struct sockaddr_in *)&addr1;
		struct sockaddr_in *in2 = (struct sockaddr_in *)&addr2;

		if(in1->sin_addr.s_addr < in2->sin_addr.s_addr) {
			ret = true;
			return ret;
		} else if(in1->sin_addr.s_addr == in2->sin_addr.s_addr) {
			if(in1->sin_port < in2->sin_port) {
				ret = true;
				return ret;
			} else {
				return ret;
			}
		} else {
			return ret;
		}
	}
};

typedef struct {
	unsigned char mac_addr[IFHWADDRLEN];
	struct sockaddr ip;
	struct sockaddr netmask;
	tun_socket_notifier *notifier;
} peer_info_t;

class settings
{
public:
	static settings *g_settings;

	settings();
	~settings();

	int get_app_settings_from_configuration(configure &cfg);
	int check_configuration();
public:
	static settings *get_instance();

	//configuration
	std::string tap_name;
	std::string ip4_config;
	std::string server_port;
	std::vector<std::string> peer_addr;

	//data
	linux_tun *tun;
	event_notifier *tap_notifier;
	std::map<int, tun_socket_notifier *> map_notifier;
	std::map<struct sockaddr, std::vector<tun_socket_notifier *>, sockaddr_less_then > map_clients;

	double value_strtod(std::string number);
	int get_time_val(struct timeval *timeval);
	std::string get_timestamp();
	int parse_args_from_configuration(int argc, char **argv);
};
#endif //_SETTINGS_H
