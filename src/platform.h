

/*================================================================
 *
 *
 *   文件名称：platform.h
 *   创 建 者：肖飞
 *   创建日期：2019年11月28日 星期四 11时36分18秒
 *   修改日期：2019年11月28日 星期四 11时38分11秒
 *   描    述：
 *
 *================================================================*/
#ifndef _PLATFORM_H
#define _PLATFORM_H
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#if defined(__FreeBSD__)
#define PLATFORM_BSD
#elif defined(__APPLE__)
#define PLATFORM_BSD
#elif defined(WIN32)
#define PLATFORM_WINDOWS
#ifdef WINVER
#if WINVER < 0x0501
#undef WINVER
#endif
#endif
#ifndef WINVER
#define WINVER 0x0501
#endif
#else
#define PLATFORM_LINUX
#endif
#endif //_PLATFORM_H
