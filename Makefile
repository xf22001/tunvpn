#
#
#================================================================
#   Copyright (C) 2019年05月21日 肖飞 All rights reserved.
#   
#   文件名称：Makefile
#   创 建 者：肖飞
#   创建日期：2019年05月21日 星期二 22时58分53秒
#   修改日期：2020年09月04日 星期五 09时22分47秒
#   描    述：
#
#================================================================
include $(default_rules)
ifneq ($(PI),)
TOOLCHAIN_PREFIX=~/work/raspberrypi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-
endif
#CC := gcc

app_name := tunvpn
c_files += src/main.cpp
c_files += src/linux_tun.cpp
c_files += src/ifconfig.cpp
c_files += src/os_util.cpp
c_files += src/settings.cpp
c_files += src/tap_notifier.cpp
c_files += src/socket_server.cpp
c_files += src/socket_client.cpp
c_files += src/request.cpp
c_files += src/tun_socket_notifier.cpp
c_files += src/console.cpp

LOCAL_CFLAGS := -pg -g -O2
LOCAL_CFLAGS += -Isrc
LOCAL_CFLAGS += -I$(top)/c/utils
LOCAL_CFLAGS += -DDEBUG
ifneq ($(PI),)
LOCAL_CFLAGS += -Ixiaofei/sysroot/usr/include
LOCAL_CFLAGS += -Ixiaofei/sysroot/usr/include/arm-linux-gnueabihf/
endif
LOCAL_LDFLAGS := 
ifneq ($(PI),)
LOCAL_LDFLAGS += --sysroot=xiaofei/sysroot
#LOCAL_LDFLAGS += -Lxiaofei/sysroot/lib
#LOCAL_LDFLAGS += -Lxiaofei/sysroot/lib/arm-linux-gnueabihf
#LOCAL_LDFLAGS += -Lxiaofei/sysroot/usr/lib
#LOCAL_LDFLAGS += -Lxiaofei/sysroot/usr/lib/arm-linux-gnueabihf
endif
ifeq ($(PI),)
LOCAL_LDFLAGS += -L$(top)/c/utils/out/lib
endif
LOCAL_LDFLAGS += -lcrypto -lssl -lz
LOCAL_LDFLAGS += -pthread -lrt
#LOCAL_LDFLAGS +=  -Wl,-Bstatic -lxiaofei -Wl,-Bdynamic
LOCAL_LDFLAGS +=  -static -lxiaofei
LOCAL_LDFLAGS += -Wl,-Map=$(out_dir)/$(app_name).map
ifneq ($(PI),)
LOCAL_DEPS += xiaofei/libssl-dev_1.1.1d-0+deb10u3+rpt1_armhf.deb
LOCAL_DEPS += xiaofei/zlib1g-dev_1.2.11.dfsg-1_armhf.deb
LOCAL_DEPS += xiaofei/libc6-dev_2.28-10+rpi1_armhf.deb
LOCAL_DEPS += xiaofei/libc6_2.28-10+rpi1_armhf.deb
$(LOCAL_DEPS) : 
	$(error "missing $@")
endif
include $(BUILD_APP)

include $(ADD_TARGET)
