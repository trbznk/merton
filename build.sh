#!/bin/bash

cc -Wall -Wextra -fPIC -shared -o libmerton.so merton.c
cc -Wall -Wextra -o merton merton.c