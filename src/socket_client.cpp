

/*================================================================
 *
 *
 *   文件名称：socket_client.cpp
 *   创 建 者：肖飞
 *   创建日期：2019年11月29日 星期五 14时02分31秒
 *   修改日期：2019年12月01日 星期日 22时07分01秒
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
	set_timeout(1, 0);
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
					l->printf("%8s %d bytes from %s\n", "received", ret, m_c->get_server_address_string().c_str());
					//l->dump((const char *)buffer, ret);
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
					rx_buffer_received += ret;
					process_message();
					ret = 0;
				} else if(ret > 0) {
					l->printf("%8s %d bytes from %s\n", "received", ret, m_c->get_server_address_string().c_str());
					//l->dump((const char *)buffer, ret);
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

int socket_client_notifier::send_request(char* request, int size)
{
	util_log *l = util_log::get_instance();
	struct sockaddr *address;
	socklen_t *addr_size;
	int ret = -1;

	address = m_c->get_server_address();
	addr_size = m_c->get_server_address_size();

	switch(m_c->get_type()) {
		case SOCK_STREAM: {
			ret = write(m_c->get_fd(), request, size);
		}
		break;

		case SOCK_DGRAM: {
			ret = sendto(m_c->get_fd(), request, size, 0, address, *addr_size);
		}
		break;

		default:
			break;
	}

	if(ret > 0) {
		l->printf("%8s %d bytes to %s\n", "send", ret, m_c->get_server_address_string().c_str());
		l->dump((const char *)request, ret);
	} else {
		l->printf("sendto %s %s\n", m_c->get_server_address_string().c_str(), strerror(errno));
		ret = -1;
	}

	return ret;
}

int socket_client_notifier::do_timeout()
{
	//util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();
	int ret = -1;

	if(settings->tun != NULL) {
		ret = chunk_sendto(FN_HELLO, settings->tun->get_tun_info(), sizeof(tun_info_t));
	}

	set_timeout(1, 0);

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
