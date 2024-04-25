#!/bin/bash

cc -Wall -Wextra -fPIC -shared -o monte.so monte.c
cc -Wall -Wextra -o monte monte.c