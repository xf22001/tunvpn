

/*================================================================
 *
 *
 *   文件名称：ifconfig.cpp
 *   创 建 者：肖飞
 *   创建日期：2019年11月28日 星期四 13时34分33秒
 *   修改日期：2019年11月28日 星期四 17时38分08秒
 *   描    述：
 *
 *================================================================*/
#include "ifconfig.h"

#include <stdio.h>

#include "util_log.h"
#include "regexp/regexp.h"

#include "os_util.h"
#include "settings.h"

#define CMD_BUFFER_SIZE 1024

static int ifconfig4_ip(std::string tap_name, std::string ip4_config)
{
	int ret;
	char cmd[CMD_BUFFER_SIZE];

	snprintf(cmd, CMD_BUFFER_SIZE, "ip link set dev \"%s\" up", tap_name.c_str());
	ret = run_cmd(cmd);

	if(ret != 0) {
		return ret;
	}

	snprintf(cmd, CMD_BUFFER_SIZE, "ip -4 addr flush dev \"%s\" scope global", tap_name.c_str());
	ret = run_cmd(cmd);

	if(ret != 0) {
		return ret;
	}

	snprintf(cmd, CMD_BUFFER_SIZE, "ip -4 addr add dev \"%s\" \"%s\" broadcast + scope global", tap_name.c_str(), ip4_config.c_str());
	ret = run_cmd(cmd);

	if(ret != 0) {
		return ret;
	}

	ret = 0;
	return ret;
}

static int ifconfig4_ifconfig(std::string tap_name, std::string ip4_config)
{
	int ret;
	char cmd[CMD_BUFFER_SIZE];
	regexp r;
	std::string ip;
	std::string prefix_mask;
	std::vector<std::string> matched_list;
	util_log *l = util_log::get_instance();
	int int_prefix_mask;
	u_uint32_bytes_t u_uint32_bytes;
	settings *settings = settings::get_instance();

	snprintf(cmd, CMD_BUFFER_SIZE, "ifconfig \"%s\" up", tap_name.c_str());
	ret = run_cmd(cmd);

	if(ret != 0) {
		return ret;
	}

	matched_list = r.match(ip4_config.c_str(), "([0-9\\.]+)/([0-9]+)");

	if(matched_list.size() != 3) {
		ret = -1;
		l->printf("matched_list.size():%d\n", matched_list.size());
		return ret;
	}

	ip = matched_list.at(1);
	prefix_mask = matched_list.at(2);


	int_prefix_mask = settings->value_strtod(prefix_mask);
	u_uint32_bytes.v = 0xffffffff;
	u_uint32_bytes.v >>= (32 - int_prefix_mask);
	u_uint32_bytes.v <<= (32 - int_prefix_mask);

	snprintf(cmd, CMD_BUFFER_SIZE, "ifconfig \"%s\" \"%s\" netmask \"%d.%d.%d.%d\"", tap_name.c_str(), ip.c_str(), 
			u_uint32_bytes.s.byte3,
			u_uint32_bytes.s.byte2,
			u_uint32_bytes.s.byte1,
			u_uint32_bytes.s.byte0);
	ret = run_cmd(cmd);

	if(ret != 0) {
		return ret;
	}

	ret = 0;

	return ret;

}

int ifconfig4(std::string tap_name, std::string ip4_config)
{
	int ret;
	ret = ifconfig4_ip(tap_name, ip4_config);

	if(ret == 0) {
		return ret;
	}

	ret = ifconfig4_ifconfig(tap_name, ip4_config);
	return ret;
}

int ifconfig6(std::string tap_name, std::string ip6_config)
{
	int ret;
	char cmd[CMD_BUFFER_SIZE];

	snprintf(cmd, CMD_BUFFER_SIZE, "ip link set dev \"%s\" up", tap_name.c_str());
	ret = run_cmd(cmd);

	if(ret != 0) {
		return ret;
	}

	snprintf(cmd, CMD_BUFFER_SIZE, "ip -6 addr flush dev \"%s\" scope global", tap_name.c_str());
	ret = run_cmd(cmd);

	if(ret != 0) {
		return 0;
	}

	snprintf(cmd, CMD_BUFFER_SIZE, "ip -6 addr add dev \"%s\" \"%s\" scope global", tap_name.c_str(), ip6_config.c_str());
	ret = run_cmd(cmd);

	if(ret != 0) {
		return 0;
	}

	return ret;
}
