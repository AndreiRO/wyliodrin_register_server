#!/bin/bash

function addsensor {
	output="$(curl -v -X POST --data "token=salut123&id=$1&type=$2" http://andreiro-server.iot.wyliodrin.com/register)"
	echo $output
}

function getsensorvalue {
	output="$(curl -v -X POST --data "token=salut123&id=$1" http://andreiro-server.iot.wyliodrin.com/get)"
	echo $output
}

function sendsensorvalue {
	output="$(curl -v -X POST --data "token=salut123&id=$1&value=$2" http://andreiro-server.iot.wyliodrin.com/send)"
	echo $output
}


if [ "$1" == "add" ]; then
	addsensor $2 $3
elif [ "$1" == "get" ]; then
	getsensorvalue $2
elif [ "$1" == "send" ]; then
	sendsensorvalue $2 $3
else
	echo "Error. Usage $0 add/get/send"
fi