

/*================================================================
 *   
 *   
 *   文件名称：socket_client.h
 *   创 建 者：肖飞
 *   创建日期：2019年11月29日 星期五 14时02分36秒
 *   修改日期：2019年11月29日 星期五 17时39分01秒
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
#include "event_loop.h"
#include "net/client.h"

class socket_client_notifier : public event_notifier
{
private:
	char tx_buffer[SOCKET_TXRX_BUFFER_SIZE];
	char rx_buffer[SOCKET_TXRX_BUFFER_SIZE];

	client *m_c;

	socket_client_notifier();
public:
	socket_client_notifier(client *c, unsigned int events = POLLIN);
	virtual ~socket_client_notifier();
	int handle_event(int fd, unsigned int events);
	int do_timeout();
};

#endif //_SOCKET_CLIENT_H
