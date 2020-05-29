

/*================================================================
 *
 *
 *   文件名称：settings.h
 *   创 建 者：肖飞
 *   创建日期：2019年11月28日 星期四 17时02分11秒
 *   修改日期：2020年05月28日 星期四 16时38分15秒
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
#include <time.h>

#ifdef __cplusplus
}
#endif

#include "configure/configure.h"
#include "linux_tun.h"
#include "tun_socket_notifier.h"
#include "lock.h"

#define CLIENT_VALIDE_TIMEOUT 15

typedef struct {
	tun_info_t tun_info;
	tun_socket_notifier *notifier;
	time_t time;
} peer_info_t;

typedef struct {
	int domain;
	struct sockaddr_storage address;
	socklen_t address_size;
} sockaddr_info_t;

struct sockaddr_info_less_then {
	bool operator() (const sockaddr_info_t &sockaddr_info1, const sockaddr_info_t &sockaddr_info2) const
	{
		if(sockaddr_info1.address_size < sockaddr_info2.address_size) {
			return true;
		}

		if(sockaddr_info1.address_size > sockaddr_info2.address_size) {
			return false;
		}

		if(memcmp(&sockaddr_info1.address, &sockaddr_info2.address, sockaddr_info1.address_size) < 0) {
			return true;
		}

		return false;
	}
};

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
	event_notifier *input_notifier;
	std::map<int, tun_socket_notifier *> map_notifier;
	std::map<int, std::string> map_host;
	std::map<sockaddr_info_t, peer_info_t, sockaddr_info_less_then> map_clients;

	mutex_lock peer_info_lock;

	double value_strtod(std::string number);
	int get_time_val(struct timeval *timeval);
	std::string get_timestamp();
	int parse_args_from_configuration(int argc, char **argv);
	void add_peer_info(int fd, tun_socket_notifier *notifier, std::string host);
	void remove_peer_info(int fd);
	bool find_peer_host(std::string host);
};
#endif //_SETTINGS_H
