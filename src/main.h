

/*================================================================
 *   
 *   
 *   文件名称：main.h
 *   创 建 者：肖飞
 *   创建日期：2019年11月28日 星期四 10时50分15秒
 *   修改日期：2019年12月01日 星期日 09时31分18秒
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
#include "tun_socket_notifier.h"

class loop_thread : public thread
{
public:
	loop_thread();
	~loop_thread();
	void func();
	void start();
};

#endif //_MAIN_H
