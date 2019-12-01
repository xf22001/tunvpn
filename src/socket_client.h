

/*================================================================
 *   
 *   
 *   文件名称：socket_client.h
 *   创 建 者：肖飞
 *   创建日期：2019年11月29日 星期五 14时02分36秒
 *   修改日期：2019年12月01日 星期日 09时35分06秒
 *   描    述：
 *
 *================================================================*/
#ifndef _SOCKET_CLIENT_H
#define _SOCKET_CLIENT_H
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#include "os_util.h"
#include "tun_socket_notifier.h"
#include "net/client.h"

class socket_client_notifier : public tun_socket_notifier
{
private:
	client *m_c;

	socket_client_notifier();
public:
	socket_client_notifier(client *c, unsigned int events = POLLIN);
	virtual ~socket_client_notifier();
	int handle_event(int fd, unsigned int events);
	int do_timeout();
};

int start_client(std::string server_address, unsigned short server_port, trans_protocol_type_t protocol);

#endif //_SOCKET_CLIENT_H