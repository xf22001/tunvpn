

/*================================================================
 *   
 *   
 *   文件名称：main.h
 *   创 建 者：肖飞
 *   创建日期：2019年11月28日 星期四 10时50分15秒
 *   修改日期：2019年11月28日 星期四 15时52分01秒
 *   描    述：
 *
 *================================================================*/
#ifndef _MAIN_H
#define _MAIN_H
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#include "thread.h"
#include "event_loop.h"

#define TAP_DATA_BUFFER_SIZE 4096

class loop_thread : public thread
{
public:
	loop_thread();
	~loop_thread();
	void func();
	void start();
};

class tap_notifier : public event_notifier
{
private:
	char buffer[TAP_DATA_BUFFER_SIZE];
	tap_notifier();
public:
	tap_notifier(int fd, unsigned int events = POLLIN);
	virtual ~tap_notifier();
	int handle_event(int fd, unsigned int events);
};

#endif //_MAIN_H
