#!/bin/bash

cp -r ~/.ssh .
docker build -t libmantle-dev .
cd ../
docker run --privileged --net host --rm -it -v `pwd`:/root -w /root libmantle-dev
