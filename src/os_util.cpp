

/*================================================================
 *
 *
 *   文件名称：os_util.cpp
 *   创 建 者：肖飞
 *   创建日期：2019年11月28日 星期四 14时58分47秒
 *   修改日期：2019年12月01日 星期日 17时02分44秒
 *   描    述：
 *
 *================================================================*/
#include "os_util.h"

#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "util_log.h"

int run_cmd(const char *cmd)
{
	util_log *l = util_log::get_instance();
	int ret = system(cmd);
	l->printf("cmd=\"%s\", result=\"%d\"\n", cmd, ret);

	return ret;
}

std::vector<std::string> get_host_by_name(std::string hostname)
{
	std::vector<std::string> hosts;
	char **cp;
	util_log *l = util_log::get_instance();


	struct hostent *ent;

	ent = gethostbyname(hostname.c_str());
	if(ent == NULL) {
		return hosts;
	}

	l->printf("h_name:%s\n", ent->h_name);
	l->printf("h_aliases:");
	cp = ent->h_aliases;

	while(*cp != NULL) {
		l->printf("%s", *cp);
		cp += 1;

		if(*cp != NULL) {
			l->printf(", ");
		}
	}

	l->printf("\n");

	l->printf("h_addrtype:%d\n", ent->h_addrtype);

	l->printf("h_length:%d\n", ent->h_length);

	l->printf("h_addr_list:");
	cp = ent->h_addr_list;

	while(*cp != NULL) {
		char buffer[32];
		const char *ip_addr = inet_ntop(ent->h_addrtype, *cp, buffer, sizeof(buffer));
		l->printf("%s", ip_addr);
		hosts.push_back(ip_addr);
		cp += 1;

		if(*cp != NULL) {
			l->printf(", ");
		}
	}

	l->printf("\n");

	return hosts;
}
