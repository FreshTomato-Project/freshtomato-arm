#!/bin/sh

ubus call uqmid add_modem '{ "name": "modem1", "device": "/dev/cdc-wdm0"}'
