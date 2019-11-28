

/*================================================================
 *
 *
 *   文件名称：settings.h
 *   创 建 者：肖飞
 *   创建日期：2019年11月28日 星期四 17时02分11秒
 *   修改日期：2019年11月28日 星期四 17时32分21秒
 *   描    述：
 *
 *================================================================*/
#ifndef _SETTINGS_H
#define _SETTINGS_H
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#include "configure/configure.h"

class settings
{
public:
	static settings *g_settings;

	settings();
	~settings();

	int get_app_settings_from_configuration(configure &cfg);
	int check_configuration();
public:
	static settings *get_instance();
	std::string tap_name;
	std::string ip4_config;

	double value_strtod(std::string number);
	int get_time_val(struct timeval *timeval);
	std::string get_timestamp();
	int parse_args_from_configuration(int argc, char **argv);
};
#endif //_SETTINGS_H
