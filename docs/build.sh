#!/bin/bash

cd ../
docker run --rm -it -v `pwd`:/root -w /root/docs --entrypoint=doxygen doxygen
cd -
