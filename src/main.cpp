

/*================================================================
 *
 *
 *   文件名称：main.cpp
 *   创 建 者：肖飞
 *   创建日期：2019年11月28日 星期四 10时49分25秒
 *   修改日期：2019年11月29日 星期五 14时06分06秒
 *   描    述：
 *
 *================================================================*/
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "util_log.h"

#include "linux_tun.h"
#include "ifconfig.h"
#include "settings.h"
#include "socket_server.h"

loop_thread::loop_thread()
{
}

loop_thread::~loop_thread()
{
}

void loop_thread::func()
{
	event_loop *loop = event_loop::get_instance();
	loop->loop();
}

void loop_thread::start()
{
	run();
}

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

int test_tun()
{
	int ret = 0;
	util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();
	linux_tun *tun = new linux_tun();

	settings->tun = tun;

	ret = tun->open_tun(settings->tap_name);

	if(ret != 0) {
		l->printf("The TAP device could not be opened! This might be caused by:\n");
		l->printf("- a missing TAP device driver,\n");
		l->printf("- a blocked TAP device (try a different name),");
		l->printf("- insufficient privileges (try running as the root/administrator user).\n");
		return ret;
	}

	settings->tap_name = tun->get_tap_name();

	ifconfig4(settings->tap_name, settings->ip4_config);

	ret = tun->get_tun_info();

	settings->socket_server_notifier = start_serve(settings->value_strtod(settings->server_port), TRANS_PROTOCOL_UDP);
	settings->tap_notifier = new tap_notifier(tun->get_tap_fd(), POLLIN);


	return ret;
}

int main(int argc, char **argv)
{
	int ret = 0;
	std::string tap_name = "tunvpn";
	//util_log *l = util_log::get_instance();
	settings *settings = settings::get_instance();

	loop_thread *th = new loop_thread;

	ret = settings->parse_args_from_configuration(argc, argv);

	if(ret != 0) {
		return ret;
	}

	th->start();

	ret = test_tun();

	if(ret != 0) {
		return ret;
	}

	th->wait();

	return ret;
}
