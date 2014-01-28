#!/bin/bash
set -e
echo "Basic testing..."
phpunit --testsuite PP --debug || exit 1

echo "Stress testing..."
phpunit --testsuite PP --repeat 100 || exit 1

# if [[ $(phpenv version-name) =~ 5.[345] ]] ; then
echo "Testing pux extension..."
cd ext
./compile -cp || exit 1
./test -- --debug || exit 1

echo "Stress testing on extension..."
./test -- --repeat 100 || exit 1
cd ..
# fi
