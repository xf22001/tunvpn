#!/bin/bash

#================================================================
#   
#   
#   文件名称：copy.sh
#   创 建 者：肖飞
#   创建日期：2019年09月27日 星期五 12时16分11秒
#   修改日期：2020年06月12日 星期五 22时37分01秒
#   描    述：
#
#================================================================
function main() {
	make clean
	make PI=1 cscope
	cp -a out/bin/tunvpn raspberry/
	#rsync -aR ./raspberry/./* pi@10.0.0.3:~/
	rsync -aR ./raspberry/./* pi@192.168.31.120:~/

	make clean
	make cscope
}

main $@
