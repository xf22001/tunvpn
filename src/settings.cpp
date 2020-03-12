

/*================================================================
 *
 *
 *   文件名称：settings.cpp
 *   创 建 者：肖飞
 *   创建日期：2019年11月28日 星期四 17时05分13秒
 *   修改日期：2020年03月12日 星期四 09时01分16秒
 *   描    述：
 *
 *================================================================*/
#include "settings.h"

#include <sys/time.h>

#include "optparse/optparse.h"
#include "util_log.h"

settings *settings::g_settings = NULL;

settings::settings()
{
	util_log *l = util_log::get_instance();
	tun = NULL;

	map_clients.clear();

	l->printf("%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

settings::~settings()
{
	util_log *l = util_log::get_instance();
	l->printf("%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

settings *settings::get_instance()
{
	if(g_settings == NULL) {
		g_settings = new settings();
	}

	return g_settings;
}

double settings::value_strtod(std::string number)
{
	double ret = 0;
	char *invalid_pos;
	util_log *l = util_log::get_instance();

	if(number.size() == 0) {
		l->printf("parameter not set!!!\n");
		exit(1);
	}

	ret = strtod(number.c_str(), &invalid_pos);

	return ret;
}

int settings::get_time_val(struct timeval *timeval)
{
	int ret = 0;
	struct timeval tv;
	struct timezone tz;

	gettimeofday(&tv, &tz);

	*timeval = tv;

	return ret;
}

std::string settings::get_timestamp()
{
	std::string ret;
	char buffer[BUFFER_LEN];
	int len = 0;
	struct tm *tm;
	struct timeval tv;

	get_time_val(&tv);

	tm = localtime(&tv.tv_sec);

	len = snprintf(buffer, BUFFER_LEN, "%04d%02d%02d%02d%02d%02d_%06d",
	               tm->tm_year + 1900,
	               tm->tm_mon + 1,
	               tm->tm_mday,
	               tm->tm_hour,
	               tm->tm_min,
	               tm->tm_sec,
	               (int)tv.tv_usec
	              );
	buffer[len] = 0;
	ret = buffer;
	return ret;
}

int settings::check_configuration()
{
	int ret = 0;
	return ret;
}

int settings::get_app_settings_from_configuration(configure &cfg)
{
	int ret = 0;

	std::vector<std::string> values;

	values = cfg.get("app", "tap_name");

	if(values.size() > 0) {
		tap_name = values.at(0);
	}

	values = cfg.get("app", "ip4_config");

	if(values.size() > 0) {
		ip4_config = values.at(0);
	}

	values = cfg.get("app", "server_port");

	if(values.size() > 0) {
		server_port = values.at(0);
	}

	values = cfg.get("app", "peer_addr");

	if(values.size() > 0) {
		peer_addr = values;
	}

	return ret;
}

int settings::parse_args_from_configuration(int argc, char **argv)
{
	int ret;
	bool have_configure_file = false;
	optparse opt;
	configure cfg;
	std::string configure_file = "default.ini";
	util_log *l = util_log::get_instance();

	opt.add_long_option("configure_file", true, false, 'f', "assign configuration file");

	ret = opt.get_long_option(argc, argv);

	if(ret != 0) {
		opt.p_help();
	} else {
		have_configure_file = true;

		if(opt.have_option("f")) {
			configure_file = opt.option_value("f");
		}
	}

	if(!have_configure_file) {
		ret = -1;
		return ret;
	}

	ret = cfg.load(configure_file);

	if(ret == 0) {
		cfg.p_configure();
		ret = get_app_settings_from_configuration(cfg);
	} else {
		l->printf("load configuration file failed!!!\n");
	}

	if(ret == 0) {
		ret = check_configuration();
	}

	return ret;
}

void settings::add_peer_info(int fd, tun_socket_notifier *notifier, std::string host)
{
	single_lock lock(&peer_info_lock);

	map_notifier[fd] = notifier;
	map_host[fd] = host;
}

void settings::remove_peer_info(int fd)
{
	single_lock lock(&peer_info_lock);

	map_notifier.erase(fd);
	map_host.erase(fd);
}

bool settings::find_peer_host(std::string host)
{
	single_lock lock(&peer_info_lock);

	std::map<int, std::string>::iterator it_host;
	bool find_host = false;

	for(it_host = map_host.begin(); it_host != map_host.end(); it_host++) {
		if(it_host->second == host) {
			find_host = true;
			break;
		}
	}

	return find_host;
}
