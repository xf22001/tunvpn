

/*================================================================
 *
 *
 *   文件名称：socket_server.h
 *   创 建 者：肖飞
 *   创建日期：2019年11月29日 星期五 11时48分31秒
 *   修改日期：2019年12月03日 星期二 14时12分47秒
 *   描    述：
 *
 *================================================================*/
#ifndef _SOCKET_SERVER_H
#define _SOCKET_SERVER_H
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif


#include "net/server.h"
#include "tun_socket_notifier.h"
#include "os_util.h"

class socket_server_client_notifier : public tun_socket_notifier
{
private:
	int m_fd;
	std::string m_address_string;
	struct sockaddr m_address;
	socket_server_client_notifier();

public:
	socket_server_client_notifier(struct sockaddr *sockaddr, std::string client_address, int fd, unsigned int events = POLLIN);
	virtual ~socket_server_client_notifier();
	int handle_event(int fd, unsigned int events);
	std::string get_request_address_string();
	struct sockaddr *get_request_address();
	void reply_tun_info();
	int send_request(char *request, int size, struct sockaddr *address, socklen_t addr_size);
};

class socket_server_notifier : public tun_socket_notifier
{
private:
	server *m_s;
	socket_server_notifier();

public:
	socket_server_notifier(server *s, unsigned int events = POLLIN);
	virtual ~socket_server_notifier();
	int handle_event(int fd, unsigned int events);
	int do_timeout();
	struct sockaddr *get_request_address();
	void reply_tun_info();
	int send_request(char *request, int size, struct sockaddr *address, socklen_t addr_size);
};

int start_serve(short server_port, trans_protocol_type_t protocol);

#endif //_SOCKET_SERVER_H
