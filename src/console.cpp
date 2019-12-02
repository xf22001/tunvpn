

/*================================================================
 *
 *
 *   文件名称：console.cpp
 *   创 建 者：肖飞
 *   创建日期：2019年12月02日 星期一 12时49分52秒
 *   修改日期：2019年12月02日 星期一 13时34分49秒
 *   描    述：
 *
 *================================================================*/
#include "console.h"

#include <string.h>
#include <unistd.h>

#include "util_log.h"
#include "settings.h"

input_notifier::input_notifier(int fd, unsigned int events) : event_notifier(fd, events)
{
	add_loop();
}

input_notifier::~input_notifier()
{
	remove_loop();
}

int input_notifier::handle_event(int fd, unsigned int events)
{
	int ret = 0;
	//util_log *l = util_log::get_instance();

	//l->printf("fd:%4d, events:%08x\n", fd, events);

	memset(input_buffer, 0x00, CONSOLE_BUFFER_SIZE);

	ret = read(fd, input_buffer, CONSOLE_BUFFER_SIZE);
	process_message(ret);

	return ret;
}

void input_notifier::process_message(int size)
{
	int ret = 0;
	util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();
	uint32_t fn;
	int catched;
	std::map<struct sockaddr, peer_info_t, sockaddr_less_then>::iterator it;

	//l->dump((const char *)input_buffer, size);

	ret = sscanf(input_buffer, "%d%n", &fn, &catched);

	if(ret == 1) {
		l->printf("fn:%d\n", fn);

		switch(fn) {
			case 0: {
				struct sockaddr remote_addr;
				peer_info_t *peer_info;
				struct sockaddr_in *sin;
				char buffer_remote[32];
				char buffer_peer[32];
				char buffer_mac[32];

				if(settings->map_clients.size() == 0) {
					break;
				}

				l->printf("%-10s\t%-10s\t%-20s\n", 
						"remote ip",
						"peer ip",
						"mac addr");

				for(it = settings->map_clients.begin(); it != settings->map_clients.end(); it++) {
					remote_addr = it->first;
					peer_info = &it->second;

					sin = (struct sockaddr_in *)&remote_addr;
					inet_ntop(AF_INET, &sin->sin_addr, buffer_remote, sizeof(buffer_remote));

					sin = (struct sockaddr_in *)&peer_info->tun_info.ip;
					inet_ntop(AF_INET, &sin->sin_addr, buffer_peer, sizeof(buffer_peer));

					snprintf(buffer_mac, 32, "%02x:%02x:%02x:%02x:%02x:%02x",
					         peer_info->tun_info.mac_addr[0],
					         peer_info->tun_info.mac_addr[1],
					         peer_info->tun_info.mac_addr[2],
					         peer_info->tun_info.mac_addr[3],
					         peer_info->tun_info.mac_addr[4],
					         peer_info->tun_info.mac_addr[5]);

					l->printf("%-10s\t%-10s\t%-20s\n",
					          buffer_remote,
					          buffer_peer,
						  buffer_mac);
				}
			}
			break;

			default: {
			}
			break;
		}
	}
}

