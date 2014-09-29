#!/bin/sh -
#
# Copyright (c) 2001, 2002
#	Interactive People Unplugged AB.  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. All advertising materials mentioning features or use of this software
#    must display the following acknowledgement:
#	This product includes software developed by Interactive People 
#	AB, Sweden and its contributors.
# 4. Neither the name of the IP Unplugged AB nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#

if [ ! -r version ]
then
	echo 0 > version
fi

touch version
v=`cat version` u=${USER-root} d=`pwd` h=`hostname` t=`env LANG=en_US.UTF-8 date`
id=`basename ${d}`
git=`git log -1|head -n 3|perl -e 'local $/;$a=<>;$a=~s/\n/ /g;print $a;'`
ost="PMON2000"
osr="3.3"

cat >vers.c <<eof
char ostype[] = "${ost}";
char osrelease[] = "${osr}";
char osversion[] = "${id}#${v}";
char sccs[8] = { ' ', ' ', ' ', ' ', '@', '(', '#', ')' };
char vers[] =
    "${ost} ${osr} (${id}) #${v}: ${t} ${git}";
eof

cat >vers.h <<eof
#define VERS "${ost} ${osr} (${id}) #${v}: ${t}\r\n"
eof

expr ${v} + 1 > version
