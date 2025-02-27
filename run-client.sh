#!/bin/sh

set +xe

./bin/spqr-client 127.0.0.1

# valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/spqr-client 127.0.0.1
