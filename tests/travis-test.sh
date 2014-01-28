#!/bin/bash
echo "Basic testing..."
phpunit --debug

echo "Stress testing..."
phpunit --repeat 100

if [[ $(phpenv version-name) =~ 5.[345] ]] ; then
    echo "Testing pux extension..."
    cd ext
    make clean && make || exit 1
    extunit --phpunit --debug || exit 1

    echo "Stress testing on extension..."
    extunit --phpunit --repeat 100 || exit 1
    cd ..
fi
