#!/bin/bash
echo "Basic testing..."
phpunit --debug || exit 1

echo "Stress testing..."
phpunit --repeat 100 || exit 1

if [[ $(phpenv version-name) =~ 5.[345] ]] ; then
    echo "Testing pux extension..."
    cd ext
    make clean && make || exit 1
    ./test -- --debug || exit 1

    echo "Stress testing on extension..."
    ./test -- --repeat 100 || exit 1
    cd ..
fi
