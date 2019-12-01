

/*================================================================
 *   
 *   
 *   文件名称：tap_notifier.cpp
 *   创 建 者：肖飞
 *   创建日期：2019年12月01日 星期日 09时29分18秒
 *   修改日期：2019年12月01日 星期日 09时39分15秒
 *   描    述：
 *
 *================================================================*/
#include "tap_notifier.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "util_log.h"

tap_notifier::tap_notifier(int fd, unsigned int events) : event_notifier(fd, events)
{
	add_loop();
}

tap_notifier::~tap_notifier()
{
	remove_loop();
}

int tap_notifier::handle_event(int fd, unsigned int events)
{
	int ret = 0;
	util_log *l = util_log::get_instance();

	if((events ^ get_events()) != 0) {
		l->printf("events:%08x\n", events);
		ret = -1;
		return ret;
	}

	ret = read(fd, buffer, TAP_DATA_BUFFER_SIZE);

	if(ret > 0) {
		//l->dump((const char *)buffer, ret);
		//ret = write(fd, buffer, ret);

		//if(ret < 0) {
		//	l->printf("write tap device error!(%s)\n", strerror(errno));
		//}
	} else {
		l->printf("read tap device error!(%s)\n", strerror(errno));
	}


	return ret;
}

