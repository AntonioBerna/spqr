#!/bin/sh

set +xe

./bin/server

# valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/server
