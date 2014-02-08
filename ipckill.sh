#!/bin/bash
# This program will kill all ipc belong your account name
# and was test in ubuntu 12.04
# by xvocker in Fri Feb  7 14:00:07 CST 2014 

PARAS="m s q"

if test -z $1 ; then
	echo "error:parameter 1 is your account name"
else
	for para in $PARAS
		do
			IDS=`ipcs -${para} | grep $1 | awk 'FS=" " {print $2}'`
			for id in $IDS
				do
					ipcrm -$para $id
				done
		done
fi
