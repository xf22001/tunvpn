

/*================================================================
 *
 *
 *   文件名称：socket_client.cpp
 *   创 建 者：肖飞
 *   创建日期：2019年11月29日 星期五 14时02分31秒
 *   修改日期：2020年09月03日 星期四 10时44分59秒
 *   描    述：
 *
 *================================================================*/
#include "socket_client.h"

#include <unistd.h>

#include "util_log.h"
#include "settings.h"
#include "main.h"

socket_client_notifier::socket_client_notifier(client *c, unsigned int events) : tun_socket_notifier(c->get_fd(), events)
{
	util_log *l = util_log::get_instance();
	l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
	m_c = c;
	update_time = time(NULL);
	add_loop(get_event_loop());
	set_timeout(1, 0);
}

socket_client_notifier::~socket_client_notifier()
{
	util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();
	l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
	remove_loop();
	settings->remove_peer_info(m_c->get_fd());
	delete m_c;
}

int socket_client_notifier::handle_event(int fd, unsigned int events)
{
	int ret = -1;
	util_log *l = util_log::get_instance();

	if((events & get_events()) == 0) {
		l->printf("client:events:%08x\n", events);
	} else {
		if((events & POLLOUT) != 0) {
			send_request_data();
		}

		if((events & POLLIN) != 0) {
			switch(m_c->get_type()) {
				case SOCK_STREAM: {
					ret = read(fd, rx_buffer + rx_buffer_received, SOCKET_TXRX_BUFFER_SIZE - rx_buffer_received);

					if(ret == 0) {
						l->printf("client read no data, may be server %s closed!\n", m_c->get_server_address_string().c_str());
						delete this;
						ret = 0;
					} else if(ret > 0) {
						//l->printf("%8s %d bytes from %s\n", "received", ret, m_c->get_server_address_string().c_str());
						rx_buffer_received += ret;
						process_message();
						update_time = time(NULL);
						ret = 0;
					} else {
						l->printf("read %s error %d(%s)\n", m_c->get_server_address_string().c_str(), ret, strerror(errno));
					}
				}
				break;

				case SOCK_DGRAM: {
					ret = recvfrom(fd, rx_buffer + rx_buffer_received, SOCKET_TXRX_BUFFER_SIZE - rx_buffer_received, 0, NULL, NULL);

					if (ret == 0) {
						l->printf("client read no data, may be server %s closed!\n", m_c->get_server_address_string().c_str());
					} else if(ret > 0) {
						//l->printf("%8s %d bytes from %s\n", "received", ret, m_c->get_server_address_string().c_str());
						rx_buffer_received += ret;
						process_message();
						update_time = time(NULL);
						ret = 0;
					} else {
						l->printf("recvfrom %s error %d(%s)\n", m_c->get_server_address_string().c_str(), ret, strerror(errno));
					}
				}
				break;

				default:
					break;
			}
		}
	}

	return ret;
}

int socket_client_notifier::send_request(char *request, int size, struct sockaddr *address, socklen_t address_size)
{
	util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();
	int ret = -1;

	switch(m_c->get_type()) {
		case SOCK_STREAM: {
			ret = write(m_c->get_fd(), request, size);
		}
		break;

		case SOCK_DGRAM: {
			ret = sendto(m_c->get_fd(), request, size, 0, address, address_size);
		}
		break;

		default:
			break;
	}

	if(ret > 0) {
		//std::string address_string = settings->get_address_string(get_domain(), address, &address_size);
		//l->printf("%8s %d bytes to %s\n", "send", ret, address_string.c_str());
	} else {
		std::string address_string = settings->get_address_string(get_domain(), address, &address_size);

		l->printf("sendto %s error %d(%s)\n", address_string.c_str(), ret, strerror(errno));
		ret = -1;
	}

	return ret;
}

struct sockaddr *socket_client_notifier::get_request_address()
{
	return m_c->get_server_address();
}

socklen_t *socket_client_notifier::get_request_address_size()
{
	return m_c->get_server_address_size();
}

int socket_client_notifier::get_domain()
{
	return m_c->get_domain();
}

void socket_client_notifier::reply_tun_info()
{
}

int socket_client_notifier::do_timeout()
{
	//util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();
	int ret = -1;

	//l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);

	if(settings->tun != NULL) {
		ret = add_request_data(FN_HELLO, settings->tun->get_tun_info(), sizeof(tun_info_t), m_c->get_server_address(), *m_c->get_server_address_size());

		if(ret > 0) {
			ret = 0;
		}
	}

	set_timeout(3, 0);

	check_client();

	if(time(NULL) - update_time >= CLIENT_VALIDE_TIMEOUT) {
		delete this;
	}

	return ret;
}

int start_client(std::string host, std::string server_port, trans_protocol_type_t protocol)
{
	int ret = -1;
	util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();
	client *c = NULL;
	tun_socket_notifier *notifier = NULL;
	std::vector<std::string> hosts;

	l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);

	if(protocol == TRANS_PROTOCOL_TCP) {
		c = new client(AF_UNSPEC, SOCK_STREAM, IPPROTO_IP, host, server_port);
	} else if(protocol == TRANS_PROTOCOL_UDP) {
		c = new client(AF_UNSPEC, SOCK_DGRAM, IPPROTO_UDP, host, server_port);
	}

	if(c == NULL) {
		return ret;
	}

	if(c->get_error()) {
		return ret;
	}

	notifier = new socket_client_notifier(c, POLLIN);
	settings->add_peer_info(c->get_fd(), notifier, host);

	ret = 0;

	return ret;
}
