

/*================================================================
 *
 *
 *   文件名称：tun_socket_notifier.h
 *   创 建 者：肖飞
 *   创建日期：2019年11月30日 星期六 22时08分15秒
 *   修改日期：2019年12月01日 星期日 15时42分43秒
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

#include "os_util.h"
#include "event_loop.h"

typedef enum {
	FN_HELLO = 0,
	FN_FRAME,
} tun_socket_fn_t;

typedef struct {
	unsigned int magic;
	unsigned int total_size;
	unsigned int data_offset;
	unsigned int data_size;
	unsigned char crc;
} header_info_t;

typedef struct {
	tun_socket_fn_t fn;
	unsigned int stage;
} payload_info_t;

typedef struct {
	header_info_t header;
	payload_info_t payload;
} request_t;

class tun_socket_notifier : public event_notifier
{
private:
	tun_socket_notifier();

protected:
	char tx_buffer[SOCKET_TXRX_BUFFER_SIZE];
	char rx_buffer[SOCKET_TXRX_BUFFER_SIZE];
	int rx_buffer_received;
public:
	tun_socket_notifier(int fd, unsigned int events = POLLIN);
	virtual ~tun_socket_notifier();
	unsigned char calc_crc8(void *data, size_t size);
	int chunk_sendto(tun_socket_fn_t fn, void *data, size_t size);
	void request_parse(char *buffer, int size, char **prequest, int *request_size);
	void process_message();
	int send_request(char *request, int size);
	void request_process(char *request, int request_size);
};

#endif //_TUN_SOCKET_NOTIFIER_H
