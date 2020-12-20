#!/bin/sh
set -e

cmoc -o rpn.bin rpn.c -lfp09
decb dskini rpn.dsk
decb copy -2b rpn.bin rpn.dsk,RPN.BIN
