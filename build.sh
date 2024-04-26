#!/bin/bash

cc -O3 -Wall -Wextra -fPIC -shared -o libmerton.so merton.c
cc -O3 -Wall -Wextra -o merton merton.c