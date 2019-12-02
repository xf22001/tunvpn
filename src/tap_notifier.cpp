

/*================================================================
 *
 *
 *   文件名称：tap_notifier.cpp
 *   创 建 者：肖飞
 *   创建日期：2019年12月01日 星期日 09时29分18秒
 *   修改日期：2019年12月02日 星期一 17时29分05秒
 *   描    述：
 *
 *================================================================*/
#include "tap_notifier.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <net/if.h>
#include <fcntl.h>

#include "util_log.h"
#include "settings.h"

tap_notifier::tap_notifier(int fd, unsigned int events) : event_notifier(fd, events)
{
	add_loop();
}

tap_notifier::~tap_notifier()
{
	remove_loop();
}

int tap_notifier::handle_event(int fd, unsigned int events)
{
	int ret = 0;
	util_log *l = util_log::get_instance();

	if((events ^ get_events()) != 0) {
		l->printf("events:%08x\n", events);
		ret = -1;
		return ret;
	}

	ret = read(fd, buffer, TAP_DATA_BUFFER_SIZE);

	if(ret > 0) {
		//l->dump((const char *)buffer, ret);
		//ret = write(fd, buffer, ret);

		//if(ret < 0) {
		//	l->printf("write tap device error!(%s)\n", strerror(errno));
		//}

		ret = send_tun_frame(buffer, ret);
	} else {
		l->printf("read tap device error!(%s)\n", strerror(errno));
	}


	return ret;
}

int tap_notifier::send_tun_frame(char *frame, int size)
{
	int ret = -1;
	//util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();
	unsigned char dest_mac_addr[IFHWADDRLEN];
	struct sockaddr dest_addr;
	peer_info_t *peer_info;
	//struct sockaddr_in *sin;
	std::map<struct sockaddr, peer_info_t, sockaddr_less_then>::iterator it;
	//char buffer[32];
	//char buffer_mac[32];
	//unsigned char *mac_data = (unsigned char *)frame;
	int found = 0;

	if(settings->map_clients.size() == 0) {
		return ret;
	}

	//snprintf(buffer_mac, 32, "%02x:%02x:%02x:%02x:%02x:%02x",
	//         mac_data[0],
	//         mac_data[1],
	//         mac_data[2],
	//         mac_data[3],
	//         mac_data[4],
	//         mac_data[5]);

	memcpy(dest_mac_addr, frame, IFHWADDRLEN);

	for(it = settings->map_clients.begin(); it != settings->map_clients.end(); it++) {
		dest_addr = it->first;
		peer_info = &it->second;

		//sin = (struct sockaddr_in *)&dest_addr;
		//inet_ntop(AF_INET, &sin->sin_addr, buffer, sizeof(buffer));

		if(memcmp(dest_mac_addr, peer_info->tun_info.mac_addr, IFHWADDRLEN) == 0) {
			//l->printf("send fram to %s, frame mac:%s\n", buffer, buffer_mac);
			ret = peer_info->notifier->chunk_sendto(FN_FRAME, frame, size, &dest_addr, sizeof(struct sockaddr));
			found = 1;
			break;
		}
	}

	if(found == 1) {
		return ret;
	}

	//broadcast
	for(it = settings->map_clients.begin(); it != settings->map_clients.end(); it++) {
		dest_addr = it->first;
		peer_info = &it->second;

		//sin = (struct sockaddr_in *)&dest_addr;
		//inet_ntop(AF_INET, &sin->sin_addr, buffer, sizeof(buffer));

		//l->printf("broadcast fram to %s, fram mac:%s\n", buffer, buffer_mac);

		ret = peer_info->notifier->chunk_sendto(FN_FRAME, frame, size, &dest_addr, sizeof(struct sockaddr));
	}

	return ret;
}
