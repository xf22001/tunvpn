

/*================================================================
 *
 *
 *   文件名称：tun_socket_notifier.cpp
 *   创 建 者：肖飞
 *   创建日期：2019年11月30日 星期六 22时08分09秒
 *   修改日期：2020年03月10日 星期二 09时36分13秒
 *   描    述：
 *
 *================================================================*/
#include "tun_socket_notifier.h"

#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "util_log.h"
#include "settings.h"

tun_socket_notifier::tun_socket_notifier(int fd, unsigned int events) : event_notifier(fd, events)
{
	//int flags;
	rx_buffer_received = 0;

	//flags = fcntl(fd, F_GETFL, 0);
	//flags |= O_NONBLOCK;
	//flags = fcntl(fd, F_SETFL, flags);
}

tun_socket_notifier::~tun_socket_notifier()
{
}

unsigned char tun_socket_notifier::calc_crc8(void *data, size_t size)
{
	unsigned char crc = 0;
	unsigned char *p = (unsigned char *)data;

	while(size > 0) {
		crc += *p;

		p++;
		size--;
	}

	return crc;
}

int tun_socket_notifier::send_request(char *request, int size, struct sockaddr *address, socklen_t addr_size)
{
	int ret = -1;
	util_log *l = util_log::get_instance();
	l->printf("invalid %s!\n", __PRETTY_FUNCTION__);
	return ret;
}

int tun_socket_notifier::chunk_sendto(tun_socket_fn_t fn, void *data, size_t size, struct sockaddr *address, socklen_t addr_size)
{
	int ret = 0;
	request_info_t request_info;

	request_info.fn = (unsigned int)fn;
	request_info.data = (const unsigned char *)data;
	request_info.size = size;
	request_info.consumed = 0;
	request_info.request = (request_t *)tx_buffer;
	request_info.request_size = 0;

	while(request_info.size > request_info.consumed) {
		::request_encode(&request_info);

		if(request_info.request_size != 0) {
			if(send_request(tx_buffer, request_info.request_size, address, addr_size) != (int)request_info.request_size) {
				ret = -1;
				break;
			}
		}
	}

	if(ret != -1) {
		ret = size;
	}

	return ret;
}

struct sockaddr *tun_socket_notifier::get_request_address()
{
	util_log *l = util_log::get_instance();
	l->printf("invalid %s!\n", __PRETTY_FUNCTION__);
	return NULL;
}

void tun_socket_notifier::reply_tun_info()
{
	util_log *l = util_log::get_instance();
	l->printf("invalid %s!\n", __PRETTY_FUNCTION__);
}

void tun_socket_notifier::request_process(request_t *request)
{
	tun_socket_fn_t fn = (tun_socket_fn_t)request->payload.fn;
	util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();

	switch(fn) {
		case FN_HELLO: {
			char buffer[32];
			tun_info_t *tun_info = (tun_info_t *)(request + 1);
			struct sockaddr_in *sin;
			peer_info_t peer_info;
			struct sockaddr client_address = *get_request_address();
			std::map<struct sockaddr, peer_info_t, sockaddr_less_then>::iterator it;
			bool log = false;

			peer_info.tun_info = *tun_info;
			peer_info.notifier = this;
			peer_info.time = time(NULL);

			it = settings->map_clients.find(client_address);

			if(it == settings->map_clients.end()) {
				log = true;
			} else {
				struct sockaddr_in *sin1 = (struct sockaddr_in *)&it->first;
				struct sockaddr_in *sin2 = (struct sockaddr_in *)&client_address;

				if(memcmp(sin1, sin2, __SOCKADDR_COMMON_SIZE + sizeof(in_port_t) + sizeof(struct in_addr)) != 0) {
					l->dump((const char *)sin1, __SOCKADDR_COMMON_SIZE + sizeof(in_port_t) + sizeof(struct in_addr));
					l->dump((const char *)sin2, __SOCKADDR_COMMON_SIZE + sizeof(in_port_t) + sizeof(struct in_addr));
					log = true;
				}

				settings->map_clients.erase(it);
			}

			settings->map_clients[client_address] = peer_info;
			reply_tun_info();

			if(log) {
				l->printf("update client!\n");
				sin = (struct sockaddr_in *)&client_address;
				inet_ntop(AF_INET, &sin->sin_addr, buffer, sizeof(buffer));
				l->printf("client_address:%s\n", buffer);

				l->dump((const char *)&tun_info->mac_addr, IFHWADDRLEN);

				sin = (struct sockaddr_in *)&tun_info->ip;
				inet_ntop(AF_INET, &sin->sin_addr, buffer, sizeof(buffer));
				l->printf("ip addr:%s\n", buffer);

				sin = (struct sockaddr_in *)&tun_info->netmask;
				inet_ntop(AF_INET, &sin->sin_addr, buffer, sizeof(buffer));
				l->printf("netmask:%s\n", buffer);
			}
		}
		break;

		case FN_FRAME: {
			char *frame = (char *)(request + 1);
			int total_size = request->header.total_size;
			int size = request->header.data_size;
			int ret = -1;
			struct ethhdr *frame_header = (struct ethhdr *)frame;
			tun_info_t *tun_info = settings->tun->get_tun_info();
			int unicast_frame = 0;

			int found = 0;
			std::map<struct sockaddr, peer_info_t, sockaddr_less_then>::iterator it;
			char buffer[32];
			char buffer_mac[32];
			struct sockaddr dest_addr;
			peer_info_t *peer_info;
			struct sockaddr_in *sin;
			struct sockaddr client_address = *get_request_address();

			if(total_size != size) {
				l->printf("total_size:%d, frame size:%d\n", total_size, size);
				break;
			}

			ret = memcmp(frame_header->h_source, tun_info->mac_addr, IFHWADDRLEN);

			if(ret == 0) {
				l->printf("drop frame:source mac addr is same as local tun if!\n");
				break;
			}

			ret = memcmp(frame_header->h_dest, tun_info->mac_addr, IFHWADDRLEN);

			if(ret == 0) {
				//l->printf("frame size:%d\n", size);
				ret = write(settings->tun->get_tap_fd(), frame, size);

				if(ret < 0) {
					l->printf("write tap device error!(%s)\n", strerror(errno));
				}

				break;
			}

			if((frame_header->h_dest[0] & 0x01) == 0x00) {
				unicast_frame = 1;
			}

			snprintf(buffer_mac, 32, "%02x:%02x:%02x:%02x:%02x:%02x",
			         frame_header->h_dest[0],
			         frame_header->h_dest[1],
			         frame_header->h_dest[2],
			         frame_header->h_dest[3],
			         frame_header->h_dest[4],
			         frame_header->h_dest[5]);

			for(it = settings->map_clients.begin(); it != settings->map_clients.end(); it++) {
				dest_addr = it->first;
				peer_info = &it->second;

				sin = (struct sockaddr_in *)&dest_addr;
				inet_ntop(AF_INET, &sin->sin_addr, buffer, sizeof(buffer));

				if(unicast_frame == 0) {
					if(it == settings->map_clients.find(client_address)) {
						l->printf("skip relay frame to source client:%s\n", buffer);
						continue;
					}

					l->printf("relay frame to %s, frame mac:%s\n", buffer, buffer_mac);
					ret = peer_info->notifier->chunk_sendto(FN_FRAME, frame, size, &dest_addr, sizeof(struct sockaddr));
				} else {
					if(memcmp(frame_header->h_dest, peer_info->tun_info.mac_addr, IFHWADDRLEN) == 0) {
						l->printf("relay frame to %s, frame mac:%s\n", buffer, buffer_mac);
						ret = peer_info->notifier->chunk_sendto(FN_FRAME, frame, size, &dest_addr, sizeof(struct sockaddr));
						found = 1;
						break;
					}
				}
			}

			if(unicast_frame == 0) {
				l->printf("write broadcast/multicast frame, frame mac:%s\n", buffer_mac);
				ret = write(settings->tun->get_tap_fd(), frame, size);

				if(ret < 0) {
					l->printf("write tap device error!(%s)\n", strerror(errno));
				}

				break;
			} else {
				if(found == 0) {
					l->printf("write unknow frame, frame mac:%s\n", buffer_mac);
					ret = write(settings->tun->get_tap_fd(), frame, size);

					if(ret < 0) {
						l->printf("write tap device error!(%s)\n", strerror(errno));
					}

					break;
				}
			}

		}
		break;

		default: {
		}
		break;
	}
}

void tun_socket_notifier::process_message()
{
	char *request = NULL;
	char *buffer = rx_buffer;
	//util_log *l = util_log::get_instance();
	int request_size = 0;
	int left = rx_buffer_received;

	//l->printf("net client got %d bytes\n", rx_buffer_received);
	//l->dump((const char *)rx_buffer, rx_buffer_received);

	while(left >= (int)sizeof(request_t)) {
		::request_decode(buffer, rx_buffer_received, &request, &request_size);
		//l->printf("got request_size %d\n", request_size);

		if(request != NULL) {//可能有效包
			if(request_size != 0) {//有效包
				request_process((request_t *)request);
				buffer += request_size;
				left -= request_size;
			} else {//还要收,退出包处理
				break;
			}
		} else {//没有有效包
			buffer += 1;
			left -= 1;
		}

		//l->printf("left %d bytes\n", left);
	}

	if(left > 0) {
		if(rx_buffer != buffer) {
			memmove(rx_buffer, buffer, left);
		}
	}

	rx_buffer_received = left;
}

int tun_socket_notifier::encrypt_request(unsigned char *in_data, int in_size, unsigned char *out_data, int *out_size)
{
	int ret = 0;
	int i;
	unsigned char mask = 0xff;

	for(i = 0; i < in_size; i++) {
		*out_data = *in_data ^ mask;
		in_data++;
		out_data++;
	}

	*out_size = in_size;
	return ret;
}

int tun_socket_notifier::decrypt_request(unsigned char *in_data, int in_size, unsigned char *out_data, int *out_size)
{
	int ret = 0;
	int i;
	unsigned char mask = 0xff;

	for(i = 0; i < in_size; i++) {
		*out_data = *in_data ^ mask;
		in_data++;
		out_data++;
	}

	*out_size = in_size;
	return ret;
}

void tun_socket_notifier::check_client()
{
	settings *settings = settings::get_instance();
	util_log *l = util_log::get_instance();
	std::map<struct sockaddr, peer_info_t, sockaddr_less_then>::iterator it;
	std::vector<struct sockaddr> invalid_address;
	std::vector<struct sockaddr>::iterator it_address;
	peer_info_t peer_info;
	struct sockaddr address;
	time_t current_time = time(NULL);

	for(it = settings->map_clients.begin(); it != settings->map_clients.end(); it++) {
		address = it->first;
		peer_info = it->second;

		if(current_time - peer_info.time >= CLIENT_VALIDE_TIMEOUT) {
			invalid_address.push_back(address);
		}
	}

	for(it_address = invalid_address.begin(); it_address != invalid_address.end(); it_address++) {
		char buffer[32];
		struct sockaddr_in *sin;

		settings->map_clients.erase(address);

		sin = (struct sockaddr_in *)&address;
		inet_ntop(AF_INET, &sin->sin_addr, buffer, sizeof(buffer));

		l->printf("remove inactive client:%s!\n", buffer);
	}
}
