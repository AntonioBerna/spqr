#!/bin/sh

set +xe

./bin/spqr-server

# valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/spqr-server
