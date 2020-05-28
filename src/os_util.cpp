

/*================================================================
 *
 *
 *   文件名称：os_util.cpp
 *   创 建 者：肖飞
 *   创建日期：2019年11月28日 星期四 14时58分47秒
 *   修改日期：2020年05月28日 星期四 15时49分24秒
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
