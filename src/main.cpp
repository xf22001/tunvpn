

/*================================================================
 *
 *
 *   文件名称：main.cpp
 *   创 建 者：肖飞
 *   创建日期：2019年11月28日 星期四 10时49分25秒
 *   修改日期：2019年12月01日 星期日 21时37分07秒
 *   描    述：
 *
 *================================================================*/
#include "main.h"

#include "util_log.h"

#include "linux_tun.h"
#include "ifconfig.h"
#include "settings.h"
#include "tap_notifier.h"
#include "socket_server.h"
#include "socket_client.h"
#include "regexp/regexp.h"

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

static void start_peer_client(trans_protocol_type_t protocol)
{
	settings *settings = settings::get_instance();
	util_log *l = util_log::get_instance();

	std::vector<std::string>::iterator it;

	for(it = settings->peer_addr.begin(); it != settings->peer_addr.end(); it++) {
		std::string ip_port = *it;
		std::string ip;
		std::string port;
		std::vector<std::string> hosts;

		std::vector<std::string> matched_list;
		std::string pattern = "^([^\\:]+)\\:([0-9]+)$";
		regexp r;

		matched_list = r.match(ip_port, pattern);

		if(matched_list.size() != 3) {
			continue;
		}

		ip = matched_list.at(1);
		port = matched_list.at(2);
		l->printf("ip:%s, port:%s\n", ip.c_str(), port.c_str());

		hosts = get_host_by_name(ip);
		if(hosts.empty()) {
			continue;
		}
		
		ip = hosts.at(0);
		start_client(ip, settings->value_strtod(port), protocol);
	}
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

	ret = tun->update_tun_info();

	start_serve(settings->value_strtod(settings->server_port), TRANS_PROTOCOL_UDP);

	start_peer_client(TRANS_PROTOCOL_UDP);

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
