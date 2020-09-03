

/*================================================================
 *
 *
 *   文件名称：socket_server.cpp
 *   创 建 者：肖飞
 *   创建日期：2019年11月29日 星期五 11时48分19秒
 *   修改日期：2020年09月03日 星期四 10时44分32秒
 *   描    述：
 *
 *================================================================*/
#include "socket_server.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "util_log.h"
#include "settings.h"
#include "main.h"

socket_server_client_notifier::socket_server_client_notifier(int domain, struct sockaddr *address, socklen_t *address_size, std::string client_address, int fd, unsigned int events) : tun_socket_notifier(fd, events)
{
	util_log *l = util_log::get_instance();
	l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);

	m_fd = fd;
	m_domain = domain;
	m_address_string = client_address;
	m_address_size = *address_size;
	memset(&m_address, 0, sizeof(m_address));
	memcpy(&m_address, address, m_address_size);

	add_loop(get_event_loop());
}

socket_server_client_notifier::~socket_server_client_notifier()
{
	util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();

	l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
	remove_loop();

	settings->map_notifier.erase(m_fd);
	close(m_fd);
	l->printf("close fd:%8d from %s\n", m_fd, m_address_string.c_str());
}

int socket_server_client_notifier::handle_event(int fd, unsigned int events)
{
	int ret = -1;
	util_log *l = util_log::get_instance();

	if((events & get_events()) == 0) {
		l->printf("error:events:%08x\n", events);
	} else {
		if((events & POLLOUT) != 0) {
			send_request_data();
		}

		if((events & POLLIN) != 0) {
			ret = read(fd, rx_buffer + rx_buffer_received, SOCKET_TXRX_BUFFER_SIZE - rx_buffer_received);

			if(ret == 0) {
				l->printf("server read no data, may be client %s closed!\n", m_address_string.c_str());
				delete this;
				ret = 0;
			} else if(ret > 0) {
				//l->printf("%8s %d bytes from %s\n", "received", ret, m_address_string.c_str());
				rx_buffer_received += ret;
				process_message();
				ret = 0;
			} else {
				l->printf("read %s error %d(%s)\n", m_address_string.c_str(), ret, strerror(errno));
			}
		}
	}

	return ret;
}

std::string socket_server_client_notifier::get_request_address_string()
{
	return m_address_string;
}

struct sockaddr *socket_server_client_notifier::get_request_address()
{
	return (struct sockaddr *)&m_address;
}

socklen_t *socket_server_client_notifier::get_request_address_size()
{
	return &m_address_size;
}

int socket_server_client_notifier::get_domain()
{
	return m_domain;
}

void socket_server_client_notifier::reply_tun_info()
{
	settings *settings = settings::get_instance();

	if(settings->tun != NULL) {
		int ret = add_request_data(FN_HELLO, settings->tun->get_tun_info(), sizeof(tun_info_t), (struct sockaddr *)&m_address, m_address_size);

		if(ret <= 0) {
		}
	}
}

int socket_server_client_notifier::send_request(char *request, int size, struct sockaddr *address, socklen_t address_size)
{
	util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();
	int ret = -1;

	ret = write(m_fd, request, size);

	if(ret > 0) {
		//std::string address_string = settings->get_address_string(get_domain(), address, &address_size);

		//l->printf("sendto %s %s\n", address_string.c_str(), strerror(errno));
	} else {
		std::string address_string = settings->get_address_string(get_domain(), address, &address_size);

		l->printf("sendto %s error %d(%s)\n", address_string.c_str(), ret, strerror(errno));
	}

	return ret;
}

socket_server_notifier::socket_server_notifier(server *s, unsigned int events) : tun_socket_notifier(s->get_fd(), events)
{
	util_log *l = util_log::get_instance();

	l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);

	m_s = s;
	add_loop(get_event_loop());

	set_timeout(1, 0);
}

socket_server_notifier::~socket_server_notifier()
{
	settings *settings = settings::get_instance();

	remove_loop();
	settings->map_notifier.erase(m_s->get_fd());
	delete m_s;
}

int socket_server_notifier::handle_event(int fd, unsigned int events)
{
	int ret = -1;
	util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();
	struct sockaddr *address = m_s->get_client_address();
	socklen_t *address_size = m_s->get_client_address_size();

	if((events & get_events()) == 0) {
		l->printf("server:events:%08x\n", events);
	} else {
		if((events & POLLOUT) != 0) {
			send_request_data();
		}

		if((events & POLLIN) != 0) {
			switch(m_s->get_type()) {
				case SOCK_STREAM: {
					int client_fd;
					tun_socket_notifier *notifier;
					std::string client_address_string;
					struct sockaddr *client_address;
					socklen_t *client_address_size;

					ret = accept(fd, address, address_size);

					if(ret < 0) {
						l->printf("accept %s\n", strerror(errno));
						return ret;
					}

					client_fd = ret;
					client_address_string = m_s->get_client_address_string();
					client_address = m_s->get_client_address();
					client_address_size = m_s->get_client_address_size();

					l->printf("accept fd:%8d from %s\n", client_fd, client_address_string.c_str());

					notifier = new socket_server_client_notifier(get_domain(), client_address, client_address_size, client_address_string, client_fd, POLLIN);
					settings->map_notifier[client_fd] = notifier;

					ret = 0;
				}
				break;

				case SOCK_DGRAM: {
					std::string client_address;

					ret = recvfrom(fd, rx_buffer + rx_buffer_received, SOCKET_TXRX_BUFFER_SIZE - rx_buffer_received, 0, address, address_size);
					client_address = m_s->get_client_address_string();

					if(ret == 0) {
						l->printf("server read no data, may be client %s closed!\n", client_address.c_str());
						ret = 0;
					} else if(ret > 0) {
						//l->printf("%8s %d bytes from %s\n", "received", ret, client_address.c_str());
						//l->dump((const char *)rx_buffer, ret);
						//ret = sendto(fd, rx_buffer, ret, 0,  address,  *address_size);

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
					} else {
						l->printf("recvfrom %s error %d(%s)\n", client_address.c_str(), ret, strerror(errno));
					}
				}
				break;

				default: {
				}
				break;
			}
		}
	}

	return ret;
}

int socket_server_notifier::send_request(char *request, int size, struct sockaddr *address, socklen_t address_size)
{
	util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();
	int ret = -1;
	std::string address_string = settings->get_address_string(get_domain(), address, &address_size);

	switch(m_s->get_type()) {
		case SOCK_STREAM: {
			l->printf("%s:%s:%d bug!!! send to connect socket!\n", __FILE__, __func__, __LINE__);
		}
		break;

		case SOCK_DGRAM: {
			ret = sendto(m_s->get_fd(), request, size, 0, address, address_size);
		}
		break;

		default:
			break;
	}

	if(ret > 0) {
		//l->printf("%8s %d bytes to %s\n", "send", ret, address_string.c_str());
	} else {
		l->printf("sendto %s %s\n", address_string.c_str(), strerror(errno));
	}

	return ret;
}

struct sockaddr *socket_server_notifier::get_request_address()
{
	return m_s->get_client_address();
}

socklen_t *socket_server_notifier::get_request_address_size()
{
	return m_s->get_client_address_size();
}

int socket_server_notifier::get_domain()
{
	return m_s->get_domain();
}


void socket_server_notifier::reply_tun_info()
{
	settings *settings = settings::get_instance();

	if(settings->tun != NULL) {
		int ret = add_request_data(FN_HELLO, settings->tun->get_tun_info(), sizeof(tun_info_t), m_s->get_client_address(), *m_s->get_client_address_size());

		if(ret <= 0) {
		}
	}
}

int socket_server_notifier::do_timeout()
{
	//util_log *l = util_log::get_instance();
	int ret = -1;

	//l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);

	set_timeout(3, 0);

	check_client();

	ret = 0;

	return ret;
}

int start_serve(std::string host, std::string server_port, trans_protocol_type_t protocol)
{
	int ret = -1;
	util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();
	server *s = NULL;
	tun_socket_notifier *notifier = NULL;

	l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);

	if(protocol == TRANS_PROTOCOL_TCP) {
		s = new server(1, AF_UNSPEC, SOCK_STREAM, IPPROTO_IP, host, server_port);
	} else if(protocol == TRANS_PROTOCOL_UDP) {
		s = new server(1, AF_UNSPEC, SOCK_DGRAM, IPPROTO_UDP, host, server_port);
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

