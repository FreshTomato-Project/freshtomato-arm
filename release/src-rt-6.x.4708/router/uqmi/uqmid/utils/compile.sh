#!/bin/sh

gcc -o gsmtap_read gsmtap_read.c -I/usr/include/libqmi-glib/ -I/usr/include/glib-2.0/ -I/usr/lib/glib-2.0/include/ -I/usr/include/libqrtr-glib/ -I../ -lqmi-glib -lglib-2.0
