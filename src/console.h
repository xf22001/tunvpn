

/*================================================================
 *   
 *   
 *   文件名称：console.h
 *   创 建 者：肖飞
 *   创建日期：2019年12月02日 星期一 12时49分56秒
 *   修改日期：2020年06月14日 星期日 10时06分13秒
 *   描    述：
 *
 *================================================================*/
#ifndef _CONSOLE_H
#define _CONSOLE_H
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#include "event_loop.h"

#define CONSOLE_BUFFER_SIZE 4096

class input_notifier : public event_notifier
{
private:
	char input_buffer[CONSOLE_BUFFER_SIZE];

	input_notifier();
public:
	input_notifier(int fd, unsigned int events = POLLIN);
	virtual ~input_notifier();
	int handle_event(int fd, unsigned int events);
	void process_message(int size);
};

#endif //_CONSOLE_H
