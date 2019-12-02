

/*================================================================
 *
 *
 *   文件名称：socket_client.cpp
 *   创 建 者：肖飞
 *   创建日期：2019年11月29日 星期五 14时02分31秒
 *   修改日期：2019年12月02日 星期一 14时37分31秒
 *   描    述：
 *
 *================================================================*/
#include "socket_client.h"

#include <unistd.h>

#include "util_log.h"
#include "settings.h"

socket_client_notifier::socket_client_notifier(client *c, unsigned int events) : tun_socket_notifier(c->get_fd(), events)
{
	util_log *l = util_log::get_instance();
	l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
	m_c = c;
	add_loop();
	set_timeout(3, 0);
}

socket_client_notifier::~socket_client_notifier()
{
	util_log *l = util_log::get_instance();
	l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
	remove_loop();
}

int socket_client_notifier::handle_event(int fd, unsigned int events)
{
	int ret = -1;
	util_log *l = util_log::get_instance();
	//settings *settings = settings::get_instance();

	if((events & get_events()) == 0) {
		l->printf("client:events:%08x\n", events);
	} else {
		switch(m_c->get_type()) {
			case SOCK_STREAM: {
				ret = read(fd, rx_buffer + rx_buffer_received, SOCKET_TXRX_BUFFER_SIZE - rx_buffer_received);

				if(ret == 0) {
					l->printf("client read no data, may be server %s closed!\n", m_c->get_server_address_string().c_str());
					//m_c->do_connect();
					//close(fd);
					//settings->map_clients.erase(fd);
					//delete this;
					ret = 0;
				} else if(ret > 0) {
					//l->printf("%8s %d bytes from %s\n", "received", ret, m_c->get_server_address_string().c_str());
					//l->dump((const char *)buffer, ret);
					decrypt_request((unsigned char *)(rx_buffer + rx_buffer_received), ret, (unsigned char *)(rx_buffer + rx_buffer_received), &ret);
					rx_buffer_received += ret;
					process_message();
				} else {
					l->printf("read %s %s\n", m_c->get_server_address_string().c_str(), strerror(errno));
				}
			}
			break;

			case SOCK_DGRAM: {
				ret = recvfrom(fd, rx_buffer + rx_buffer_received, SOCKET_TXRX_BUFFER_SIZE - rx_buffer_received, 0, NULL, NULL);

				if (ret == 0) {
					l->printf("client read no data, may be server %s closed!\n", m_c->get_server_address_string().c_str());
					//close(fd);
					//settings->map_clients.erase(fd);
					//delete this;
				} else if(ret > 0) {
					//l->printf("%8s %d bytes from %s\n", "received", ret, m_c->get_server_address_string().c_str());
					//l->dump((const char *)buffer, ret);
					decrypt_request((unsigned char *)(rx_buffer + rx_buffer_received), ret, (unsigned char *)(rx_buffer + rx_buffer_received), &ret);
					rx_buffer_received += ret;
					process_message();
					ret = 0;
				} else {
					l->printf("recvfrom %s %s\n", m_c->get_server_address_string().c_str(), strerror(errno));
				}
			}
			break;

			default:
				break;
		}
	}

	return ret;
}

int socket_client_notifier::send_request(char *request, int size, struct sockaddr *address, socklen_t addr_size)
{
	util_log *l = util_log::get_instance();
	struct sockaddr_in *sin = (struct sockaddr_in *)address;
	char buffer[32];
	int ret = -1;

	inet_ntop(AF_INET, &sin->sin_addr, buffer, sizeof(buffer));
	encrypt_request((unsigned char *)request, size, (unsigned char *)request, &size);

	switch(m_c->get_type()) {
		case SOCK_STREAM: {
			ret = write(m_c->get_fd(), request, size);
		}
		break;

		case SOCK_DGRAM: {
			ret = sendto(m_c->get_fd(), request, size, 0, address, addr_size);
		}
		break;

		default:
			break;
	}

	if(ret > 0) {
		//l->printf("%8s %d bytes to %s:%d\n", "send", ret, buffer, htons(sin->sin_port));
		//l->dump((const char *)request, ret);
	} else {
		l->printf("sendto %s:%d %s\n", buffer, htons(sin->sin_port), strerror(errno));
		ret = -1;
	}

	return ret;
}

void socket_client_notifier::request_process(request_t *request)
{
	tun_socket_fn_t fn = request->payload.fn;
	util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();

	switch(fn) {
		case FN_HELLO: {
			char buffer[32];
			tun_info_t *tun_info = (tun_info_t *)(request + 1);
			struct sockaddr_in *sin;
			peer_info_t peer_info;
			struct sockaddr client_address = *m_c->get_server_address();

			peer_info.tun_info = *tun_info;
			peer_info.notifier = this;

			settings->map_clients.erase(client_address);
			settings->map_clients[client_address] = peer_info;

			//l->printf("server addr:%s\n", m_c->get_server_address_string().c_str());

			//l->dump((const char *)&tun_info->mac_addr, IFHWADDRLEN);

			sin = (struct sockaddr_in *)&tun_info->ip;
			inet_ntop(AF_INET, &sin->sin_addr, buffer, sizeof(buffer));
			//l->printf("ip addr:%s\n", buffer);

			sin = (struct sockaddr_in *)&tun_info->netmask;
			inet_ntop(AF_INET, &sin->sin_addr, buffer, sizeof(buffer));
			//l->printf("netmask:%s\n", buffer);

			//if(settings->tun != NULL) {
			//	int ret = chunk_sendto(FN_HELLO, settings->tun->get_tun_info(), sizeof(tun_info_t), m_c->get_server_address(), *m_c->get_server_address_size());

			//	if(ret <= 0) {
			//	}
			//}
		}
		break;

		case FN_FRAME: {
			char *frame = (char *)(request + 1);
			int size = request->header.data_size;

			int ret = write(settings->tun->get_tap_fd(), frame, size);

			if(ret < 0) {
				l->printf("write tap device error!(%s)\n", strerror(errno));
			}
		}
		break;

		default: {
		}
		break;
	}
}


int socket_client_notifier::do_timeout()
{
	//util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();
	int ret = -1;

	if(settings->tun != NULL) {
		ret = chunk_sendto(FN_HELLO, settings->tun->get_tun_info(), sizeof(tun_info_t), m_c->get_server_address(), *m_c->get_server_address_size());
	}

	set_timeout(3, 0);

	return ret;
}

int start_client(std::string server_address, unsigned short server_port, trans_protocol_type_t protocol)
{
	int ret = -1;
	util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();
	client *c = NULL;
	tun_socket_notifier *notifier = NULL;

	l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);

	if(protocol == TRANS_PROTOCOL_TCP) {
		c = new client(AF_INET, SOCK_STREAM, IPPROTO_IP, server_address, server_port);
	} else if(protocol == TRANS_PROTOCOL_UDP) {
		c = new client(AF_INET, SOCK_DGRAM, IPPROTO_UDP, server_address, server_port);
	}

	if(c == NULL) {
		return ret;
	}

	if(c->get_error()) {
		return ret;
	}

	notifier = new socket_client_notifier(c, POLLIN);
	settings->map_notifier[c->get_fd()] = notifier;

	ret = 0;

	return ret;
}
