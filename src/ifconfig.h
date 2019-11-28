

/*================================================================
 *   
 *   
 *   文件名称：ifconfig.h
 *   创 建 者：肖飞
 *   创建日期：2019年11月28日 星期四 13时34分40秒
 *   修改日期：2019年11月28日 星期四 14时31分14秒
 *   描    述：
 *
 *================================================================*/
#ifndef _IFCONFIG_H
#define _IFCONFIG_H
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif
#include <string>

int ifconfig4(std::string tap_name, std::string ip4_config);
int ifconfig6(std::string tap_name, std::string ip6_config);
#endif //_IFCONFIG_H
