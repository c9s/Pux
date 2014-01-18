#!/bin/bash
echo "Basic testing..."
phpunit --debug

echo "Stress testing..."
phpunit --repeat 100

if [[ $(phpenv version-name) =~ 5.[345] ]] ; then
    echo "Testing pux extension..."
    cd ext
    make clean && make || exit 1
    extunit --phpunit --debug MuxTest.php || exit 1
    extunit --phpunit --debug ControllerTest.php || exit 1
    extunit --phpunit --debug PersistentMuxTest.php || exit 1

    echo "Stress testing on extension..."
    extunit --phpunit --repeat 100 MuxTest.php || exit 1
    extunit --phpunit --repeat 100 ControllerTest.php || exit 1
    extunit --phpunit --repeat 100 PersistentMuxTest.php || exit 1
    cd ..
fi
