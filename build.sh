#!/bin/bash
#本脚本用于编译龙芯1C的pmon
#本脚本在debian5-debian12, 使用debian的 交叉编译工具 进行pmon编译
#在win10的ubuntu子系统编译，也是可以的

if ! [ "$1" ] ; then
echo $0 zloader.xxx
exit
fi
dir=$1
if ! [ -x $1 ] ; then
dir="zloader.$1"
if ! [ -x $dir ] ; then
echo not find $dir 
exit
fi
fi
if ! [ "`which mipsel-linux-gnu-gcc`" ] || ! [ -e /var/log/pmon_install.txt ] ;then
apt-get update
apt-get -y install gcc-mipsel-linux-gnu
if ! [ "`which mipsel-linux-gnu-gcc`" ] ;then
echo 没有找到gcc-mipsel-linux-gnu包
exit
fi
apt-get -y install zlib1g  make bison flex xutils-dev ccache
date > /var/log/pmon_install.txt
fi
PATH=`pwd`/ccache:`pwd`/tools/pmoncfg:$PATH

if ! [ "`which pmoncfg`" ] ; then
cd tools/pmoncfg
make CPPFLAGS=-fcommon
cd ../..
fi



cd $dir
make cfg all tgt=rom CROSS_COMPILE=mipsel-linux-gnu- LANG=C
cp gzrom.bin ..
