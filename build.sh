#!/bin/sh
set -e -x

cmoc -DDOUBLE -o rpn-d.bin rpn.c -lfp09
cmoc -o rpn-s.bin rpn.c -lfp09
decb dskini rpn.dsk
decb copy -2b rpn-s.bin rpn.dsk,RPNS.BIN
decb copy -2b rpn-d.bin rpn.dsk,RPND.BIN
