#!/bin/bash

cd ../
docker run --rm -it -v `pwd`:/root -w /root --entrypoint=/bin/bash doxygen
