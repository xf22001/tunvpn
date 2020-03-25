

/*================================================================
 *
 *
 *   文件名称：tun_socket_notifier.h
 *   创 建 者：肖飞
 *   创建日期：2019年11月30日 星期六 22时08分15秒
 *   修改日期：2020年03月25日 星期三 13时06分22秒
 *   描    述：
 *
 *================================================================*/
#ifndef _TUN_SOCKET_NOTIFIER_H
#define _TUN_SOCKET_NOTIFIER_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>

#ifdef __cplusplus
}
#endif

#include <queue>

#include "os_util.h"
#include "event_loop.h"
#include "request.h"

#define MAX_REQUEST_PACKET_SIZE SOCKET_TXRX_BUFFER_SIZE

#define DEFAULT_REQUEST_MAGIC 0xa5a55a5a

typedef enum {
	FN_HELLO = 0,
	FN_FRAME,
} tun_socket_fn_t;

typedef struct {
	tun_socket_fn_t fn;
	unsigned char *data;
	size_t size;
	struct sockaddr address;
} request_data_t;

class tun_socket_notifier : public event_notifier
{
private:
	tun_socket_notifier();
	std::queue<request_data_t> queue_request_data;
	unsigned int m_init_events;

protected:
	char rx_reqest[MAX_REQUEST_PACKET_SIZE];
	char tx_buffer[SOCKET_TXRX_BUFFER_SIZE];
	char rx_buffer[SOCKET_TXRX_BUFFER_SIZE];
	int rx_buffer_received;
public:
	tun_socket_notifier(int fd, unsigned int events = POLLIN);
	virtual ~tun_socket_notifier();
	unsigned char calc_crc8(void *data, size_t size);
	int add_request_data(tun_socket_fn_t fn, void *data, size_t size, struct sockaddr *address, socklen_t addr_size);
	int send_request_data();
	int chunk_sendto(tun_socket_fn_t fn, void *data, size_t size, struct sockaddr *address, socklen_t addr_size);
	void process_message();
	int encrypt_request(unsigned char *in_data, int in_size, unsigned char *out_data, int *out_size);
	int decrypt_request(unsigned char *in_data, int in_size, unsigned char *out_data, int *out_size);
	void request_process(request_t *request);
	void check_client();
	virtual void reply_tun_info();
	virtual struct sockaddr *get_request_address();
	virtual int send_request(char *request, int size, struct sockaddr *address, socklen_t addr_size);
};

#endif //_TUN_SOCKET_NOTIFIER_H
