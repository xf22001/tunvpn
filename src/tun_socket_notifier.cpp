

/*================================================================
 *
 *
 *   文件名称：tun_socket_notifier.cpp
 *   创 建 者：肖飞
 *   创建日期：2019年11月30日 星期六 22时08分09秒
 *   修改日期：2019年12月01日 星期日 22时15分28秒
 *   描    述：
 *
 *================================================================*/
#include "tun_socket_notifier.h"

#include <string.h>

#include "util_log.h"

tun_socket_notifier::tun_socket_notifier(int fd, unsigned int events) : event_notifier(fd, events)
{
	rx_buffer_received = 0;
}

tun_socket_notifier::~tun_socket_notifier()
{
}

unsigned char tun_socket_notifier::calc_crc8(void *data, size_t size)
{
	unsigned char crc = 0;
	unsigned char *p = (unsigned char *)data;

	while(size > 0) {
		crc += *p;

		p++;
		size--;
	}

	return crc;
}

int tun_socket_notifier::send_request(char *request, int size)
{
	int ret = -1;
	return ret;
}

int tun_socket_notifier::chunk_sendto(tun_socket_fn_t fn, void *data, size_t size)
{
	size_t ret = 0;
	char *p = (char *)data;
	size_t sent = 0;
	size_t head_size = sizeof(request_t);
	size_t payload = SOCKET_TXRX_BUFFER_SIZE - head_size;

	if(data == NULL) {
		ret = -1;
		return ret;
	}

	while(sent < size) {
		size_t left = size - sent;
		size_t len = (left > payload) ? payload : left;
		request_t *request = (request_t *)tx_buffer;
		int send_size = len + head_size;
		int count;

		request->header.magic = 0xa5a55a5a;
		request->header.total_size = size;
		request->header.data_offset = sent;
		request->header.data_size = len;

		request->payload.fn = fn;
		request->payload.stage = 0;
		memcpy(request + 1, p + sent, len);

		request->header.crc = calc_crc8(((header_info_t *)&request->header) + 1, len + sizeof(payload_info_t));

		count = send_request((char *)request, send_size);

		if(count == send_size) {
			sent += len;
		} else {
			ret = -1;
			break;
		}
	}

	if(ret == 0) {
		ret = sent;
	}

	return ret;
}


void tun_socket_notifier::request_parse(char *buffer, int size, char **prequest, int *request_size)
{
	util_log *l = util_log::get_instance();
	request_t *request = (request_t *)buffer;
	size_t valid_size = size;
	size_t payload;
	size_t max_payload;

	*prequest = NULL;
	*request_size = 0;

	if(valid_size < sizeof(request_t)) {
		return;
	}

	payload = valid_size - sizeof(request_t);
	max_payload = SOCKET_TXRX_BUFFER_SIZE - sizeof(request_t);

	if(request->header.magic != 0xa5a55a5a) {//无效
		l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
		return;
	}

	if(request->header.data_size > max_payload) {//无效
		l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
		return;
	}

	if(request->header.data_size > payload) {//还要收
		l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
		*prequest = (char *)request;
		return;
	}

	if(request->header.crc != calc_crc8(((header_info_t *)&request->header) + 1, request->header.data_size + sizeof(payload_info_t))) {//无效
		l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
		*prequest = NULL;
		return;
	}

	*prequest = (char *)request;
	*request_size = request->header.data_size + sizeof(request_t);
}

void tun_socket_notifier::request_process(char *request, int request_size)
{
}

void tun_socket_notifier::process_message()
{
	char *request = NULL;
	char *buffer = rx_buffer;
	util_log *l = util_log::get_instance();
	int request_size = 0;
	int left = rx_buffer_received;

	l->printf("net client got %d bytes\n", rx_buffer_received);
	l->dump((const char *)buffer, rx_buffer_received);

	while(left >= (int)sizeof(request_t)) {
		request_parse(buffer, rx_buffer_received, &request, &request_size);

		if(request != NULL) {//可能有效包
			if(request_size != 0) {//有效包
				request_process(request, request_size);
				buffer += request_size;
				left -= request_size;
			} else {//还要收,退出包处理
				break;
			}
		} else {//没有有效包
			buffer += 1;
			left -= 1;
		}

		l->printf("request_size %d bytes\n", request_size);
		l->printf("left %d bytes\n", left);
	}

	if(left > 0) {
		if(rx_buffer != buffer) {
			memmove(rx_buffer, buffer, left);
		}
	}

	rx_buffer_received = left;
}
