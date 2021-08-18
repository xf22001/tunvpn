

/*================================================================
 *
 *
 *   文件名称：tun_socket_notifier.h
 *   创 建 者：肖飞
 *   创建日期：2019年11月30日 星期六 22时08分15秒
 *   修改日期：2021年08月18日 星期三 22时37分06秒
 *   描    述：
 *
 *================================================================*/
#ifndef _TUN_SOCKET_NOTIFIER_H
#define _TUN_SOCKET_NOTIFIER_H
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>

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
	struct sockaddr_storage address;
	socklen_t address_size;
} request_data_t;

class tun_socket_notifier : public event_notifier
{
private:
	tun_socket_notifier();
	std::queue<request_data_t> queue_request_data;
	unsigned int m_init_events;

protected:
	char tx_buffer[SOCKET_TXRX_BUFFER_SIZE];
	char rx_buffer[SOCKET_TXRX_BUFFER_SIZE];
	int rx_buffer_received;

	char decode_encode_buffer[SOCKET_TXRX_BUFFER_SIZE];
public:
	tun_socket_notifier(int fd, unsigned int events = POLLIN);
	virtual ~tun_socket_notifier();
	unsigned char calc_crc8(void *data, size_t size);
	int add_request_data(tun_socket_fn_t fn, void *data, size_t size, struct sockaddr *address, socklen_t address_size);
	int send_request_data();
	int chunk_sendto(tun_socket_fn_t fn, void *data, size_t size, struct sockaddr *address, socklen_t address_size);
	void process_message();
	int encrypt_request(unsigned char *in_data, unsigned int in_size, unsigned char *out_data, unsigned int *out_size);
	int decrypt_request(unsigned char *in_data, unsigned int in_size, unsigned char *out_data, unsigned int *out_size);
	void request_process_hello(unsigned char *data, unsigned int size);
	void request_process_frame(unsigned char *data, unsigned int size);
	void request_process(request_t *request);
	void check_client();
	virtual void reply_tun_info();
	virtual struct sockaddr *get_request_address();
	virtual socklen_t *get_request_address_size();
	virtual int get_domain();
	virtual int send_request(char *request, int size, struct sockaddr *address, socklen_t address_size);
};

#endif //_TUN_SOCKET_NOTIFIER_H
