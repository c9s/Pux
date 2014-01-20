#!/bin/bash
echo "===> Cleaning..."
phpize --clean > /dev/null
echo "===> Scaffolding..."
phpize || exit 1
echo "===> Configuring..."
./configure > /dev/null || exit 1
echo "===> Cleaning..."
make clean || exit 1
echo "===> Building..."
make clean > /dev/null && make > /dev/null && make install
