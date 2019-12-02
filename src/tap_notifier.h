

/*================================================================
 *
 *
 *   文件名称：tap_notifier.h
 *   创 建 者：肖飞
 *   创建日期：2019年12月01日 星期日 09时29分57秒
 *   修改日期：2019年12月02日 星期一 12时31分10秒
 *   描    述：
 *
 *================================================================*/
#ifndef _TAP_NOTIFIER_H
#define _TAP_NOTIFIER_H
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#include "event_loop.h"

#define TAP_DATA_BUFFER_SIZE 4096

class tap_notifier : public event_notifier
{
private:
	char buffer[TAP_DATA_BUFFER_SIZE];
	tap_notifier();
public:
	tap_notifier(int fd, unsigned int events = POLLIN);
	virtual ~tap_notifier();
	int handle_event(int fd, unsigned int events);
	int send_tun_frame(char *frame, int size);
};

#endif //_TAP_NOTIFIER_H
