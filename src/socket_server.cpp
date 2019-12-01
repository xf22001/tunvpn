

/*================================================================
 *
 *
 *   文件名称：socket_server.cpp
 *   创 建 者：肖飞
 *   创建日期：2019年11月29日 星期五 11时48分19秒
 *   修改日期：2019年12月01日 星期日 22时03分25秒
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
			l->printf("%8s %d bytes from %s\n", "received", ret, m_address.c_str());
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
					l->printf("%8s %d bytes from %s\n", "received", ret, client_address.c_str());
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

