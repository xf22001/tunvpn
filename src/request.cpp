

/*================================================================
 *
 *
 *   文件名称：request.cpp
 *   创 建 者：肖飞
 *   创建日期：2020年02月16日 星期日 11时05分58秒
 *   修改日期：2021年04月20日 星期二 10时39分47秒
 *   描    述：
 *
 *================================================================*/
#include "request.h"

#include <string.h>

#include "util_log.h"

static unsigned char request_calc_crc8(void *data, size_t size)
{
	unsigned char crc = 0;
	unsigned char *p = (unsigned char *)data;
	int i;

	for(i = 0; i < size; i++) {
		crc += p[i];
	}

	return crc;
}

void request_decode(char *buffer, int size, char **prequest, int *request_size)
{
	util_log *l = util_log::get_instance();
	request_t *request = (request_t *)buffer;
	size_t valid_size = size;
	size_t payload_size;
	size_t max_payload;

	*prequest = NULL;
	*request_size = 0;

	if(valid_size < sizeof(request_t)) {
		return;
	}

	payload_size = valid_size - sizeof(request_t);
	max_payload = SOCKET_TXRX_BUFFER_SIZE - sizeof(request_t);

	if(request->header.magic != DEFAULT_REQUEST_MAGIC) {//无效
		//l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
		l->printf("magic invalid!\n");
		return;
	}

	if(request->header.data_size > max_payload) {//无效
		//l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
		l->printf("size > max_payload invalid!\n");
		return;
	}

	if(request->header.data_size > payload_size) {//还要收
		//l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
		*prequest = (char *)request;
		return;
	}

	if(request->header.crc != request_calc_crc8(((header_info_t *)&request->header) + 1, request->header.data_size + sizeof(payload_info_t))) {//无效
		//l->printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
		l->printf("crc invalid!\n");
		*prequest = NULL;
		return;
	}

	*prequest = (char *)request;
	*request_size = request->header.data_size + sizeof(request_t);
}

void request_encode(request_info_t *request_info)
{
	//util_log *l = util_log::get_instance();
	size_t head_size = sizeof(request_t);
	size_t payload_size = MAX_REQUEST_PACKET_SIZE - head_size;
	size_t left = request_info->size - request_info->consumed;
	request_t *request = request_info->request;

	request_info->request_size = 0;

	if(request_info->data == NULL) {
		return;
	}

	if(request_info->request == NULL) {
		return;
	}

	if(left == 0) {
		return;
	}

	payload_size = (left > payload_size) ? payload_size : left;
	request_info->request_size = payload_size + head_size;

	request->header.magic = DEFAULT_REQUEST_MAGIC;
	request->header.total_size = request_info->size;
	request->header.data_size = payload_size;
	request->payload.fn = request_info->fn;
	memcpy(request + 1, (const char *)request_info->data + request_info->consumed, payload_size);
	request->header.data_offset = request_info->consumed;
	request->header.crc = request_calc_crc8(((header_info_t *)&request->header) + 1, payload_size + sizeof(payload_info_t));
	request_info->consumed += payload_size;

	//l->printf("request_size:%d!\n", request_info->request_size);
}
