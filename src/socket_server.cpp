

/*================================================================
 *
 *
 *   文件名称：socket_server.cpp
 *   创 建 者：肖飞
 *   创建日期：2019年11月29日 星期五 11时48分19秒
 *   修改日期：2019年12月02日 星期一 17时31分45秒
 *   描    述：
 *
 *================================================================*/
#include "socket_server.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "util_log.h"
#include "settings.h"

socket_server_client_notifier::socket_server_client_notifier(std::string client_address, int fd, unsigned int events) : tun_socket_notifier(fd, events)
{
	util_log *l = util_log::get_instance();
	l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);

	m_address = client_address;
	add_loop();
}

socket_server_client_notifier::~socket_server_client_notifier()
{
	util_log *l = util_log::get_instance();
	l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
	remove_loop();
}

int socket_server_client_notifier::handle_event(int fd, unsigned int events)
{
	int ret = -1;
	util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();

	if((events ^ get_events()) != 0) {
		l->printf("error:events:%08x\n", events);
		close(fd);
		settings->map_notifier.erase(fd);
		delete this;
	} else if((events & get_events()) != 0) {
		ret = read(fd, rx_buffer + rx_buffer_received, SOCKET_TXRX_BUFFER_SIZE - rx_buffer_received);

		if(ret == 0) {
			l->printf("server read no data, may be client %s closed!\n", m_address.c_str());
			close(fd);
			settings->map_notifier.erase(fd);
			delete this;
			ret = 0;
		} else if(ret > 0) {
			//l->printf("%8s %d bytes from %s\n", "received", ret, m_address.c_str());
			decrypt_request((unsigned char *)(rx_buffer + rx_buffer_received), ret, (unsigned char *)(rx_buffer + rx_buffer_received), &ret);
			//l->dump((const char *)rx_buffer, ret);
			//ret = write(fd, rx_buffer, ret);

			//if(ret <= 0) {
			//	l->printf("write %s\n", strerror(errno));
			//} else {
			//	l->printf("%8s %d bytes to %s\n", "send", ret, m_address.c_str());
			//	l->dump((const char *)rx_buffer, ret);
			//}

			rx_buffer_received += ret;
			process_message();
			ret = 0;
		}
	} else {
		l->printf("error:events:%08x\n", events);
	}

	return ret;
}

std::string socket_server_client_notifier::get_client_address_string()
{
	return m_address;
}


socket_server_notifier::socket_server_notifier(server *s, unsigned int events) : tun_socket_notifier(s->get_fd(), events)
{
	util_log *l = util_log::get_instance();

	l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);

	m_s = s;
	add_loop();
}

socket_server_notifier::~socket_server_notifier()
{
	remove_loop();
}

int socket_server_notifier::handle_event(int fd, unsigned int events)
{
	int ret = -1;
	util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();
	struct sockaddr *address = m_s->get_client_address();
	socklen_t *addr_size = m_s->get_client_address_size();

	if((events & get_events()) == 0) {
		l->printf("server:events:%08x\n", events);
	} else {
		switch(m_s->get_type()) {
			case SOCK_STREAM: {
				int client_fd;
				tun_socket_notifier *notifier;
				std::string client_address;

				ret = accept(fd, address, addr_size);

				if(ret < 0) {
					l->printf("accept %s\n", strerror(errno));
					return ret;
				}

				client_fd = ret;
				client_address = m_s->get_client_address_string();

				l->printf("accept fd:%8d from %s\n", client_fd, client_address.c_str());

				notifier = new socket_server_client_notifier(client_address, client_fd, POLLIN);
				settings->map_notifier[client_fd] = notifier;

				ret = 0;
			}
			break;

			case SOCK_DGRAM: {
				std::string client_address;

				ret = recvfrom(fd, rx_buffer + rx_buffer_received, SOCKET_TXRX_BUFFER_SIZE - rx_buffer_received, 0, address, addr_size);
				client_address = m_s->get_client_address_string();

				if(ret == 0) {
					l->printf("server read no data, may be client %s closed!\n", client_address.c_str());
					//close(fd);
					//settings->map_notifier.erase(fd);
					//delete this;
					//try reconnect~~~
					ret = 0;
				} else if(ret > 0) {
					//l->printf("%8s %d bytes from %s\n", "received", ret, client_address.c_str());
					decrypt_request((unsigned char *)(rx_buffer + rx_buffer_received), ret, (unsigned char *)(rx_buffer + rx_buffer_received), &ret);
					//l->dump((const char *)rx_buffer, ret);
					//ret = sendto(fd, rx_buffer, ret, 0,  address,  *addr_size);

					//if(ret > 0) {
					//	l->printf("%8s %d bytes to %s\n", "send", ret, client_address.c_str());
					//	l->dump((const char *)rx_buffer, ret);
					//	ret = 0;
					//} else {
					//	l->printf("sendto %s %s\n", client_address.c_str(), strerror(errno));
					//}
					rx_buffer_received += ret;
					process_message();

					ret = 0;
				}
			}
			break;

			default: {
			}
			break;
		}
	}

	return ret;
}

int socket_server_notifier::send_request(char *request, int size, struct sockaddr *address, socklen_t addr_size)
{
	util_log *l = util_log::get_instance();
	struct sockaddr_in *sin = (struct sockaddr_in *)address;
	char buffer[32];
	int ret = -1;

	inet_ntop(AF_INET, &sin->sin_addr, buffer, sizeof(buffer));
	encrypt_request((unsigned char *)request, size, (unsigned char *)request, &size);

	switch(m_s->get_type()) {
		case SOCK_STREAM: {
			ret = write(m_s->get_fd(), request, size);
		}
		break;

		case SOCK_DGRAM: {
			ret = sendto(m_s->get_fd(), request, size, 0, address, addr_size);
		}
		break;

		default:
			break;
	}

	if(ret > 0) {
		//l->printf("%8s %d bytes to %s:%d\n", "send", ret, buffer, htons(sin->sin_port));
	} else {
		l->printf("sendto %s:%d %s\n", buffer, htons(sin->sin_port), strerror(errno));
	}

	return ret;
}

void socket_server_notifier::request_process(request_t *request)
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
			struct sockaddr client_address = *m_s->get_client_address();

			peer_info.tun_info = *tun_info;
			peer_info.notifier = this;

			settings->map_clients.erase(client_address);
			settings->map_clients[client_address] = peer_info;

			//l->printf("client addr:%s\n", m_s->get_client_address_string().c_str());

			//l->dump((const char *)&tun_info->mac_addr, IFHWADDRLEN);

			sin = (struct sockaddr_in *)&tun_info->ip;
			inet_ntop(AF_INET, &sin->sin_addr, buffer, sizeof(buffer));
			//l->printf("ip addr:%s\n", buffer);

			sin = (struct sockaddr_in *)&tun_info->netmask;
			inet_ntop(AF_INET, &sin->sin_addr, buffer, sizeof(buffer));

			//l->printf("netmask:%s\n", buffer);

			if(settings->tun != NULL) {
				int ret = chunk_sendto(FN_HELLO, settings->tun->get_tun_info(), sizeof(tun_info_t), m_s->get_client_address(), *m_s->get_client_address_size());

				if(ret <= 0) {
				}
			}
		}
		break;

		case FN_FRAME: {
			char *frame = (char *)(request + 1);
			int size = request->header.data_size;
			int ret = -1;

			if(request->header.data_size != request->header.total_size) {
				if(request->header.data_offset == 0) {
					rx_seq = request->payload.seq;
				} else {
					if(rx_seq != request->payload.seq) {
						//l->printf("drop packet %d!\n", request->payload.seq);
						break;
					}
				}

				memcpy(rx_reqest + request->header.data_offset, frame, size);

				if(request->header.data_offset + request->header.data_size == request->header.total_size) {
					ret = write(settings->tun->get_tap_fd(), rx_reqest, request->header.total_size);
				} else {
					break;
				}
			} else {
				ret = write(settings->tun->get_tap_fd(), frame, size);
			}

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

int start_serve(short server_port, trans_protocol_type_t protocol)
{
	int ret = -1;
	util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();
	server *s = NULL;
	tun_socket_notifier *notifier = NULL;

	l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);

	if(protocol == TRANS_PROTOCOL_TCP) {
		s = new server(1, AF_INET, SOCK_STREAM, IPPROTO_IP, "0.0.0.0", server_port);
	} else if(protocol == TRANS_PROTOCOL_UDP) {
		s = new server(1, AF_INET, SOCK_DGRAM, IPPROTO_UDP, "0.0.0.0", server_port);
	}

	if(s == NULL) {
		return ret;
	}

	if(s->get_error()) {
		return ret;
	}

	notifier = new socket_server_notifier(s, POLLIN);

	settings->map_notifier[s->get_fd()] = notifier;

	ret = 0;

	return ret;
}

