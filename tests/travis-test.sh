#!/bin/bash
echo "Basic testing..."
phpunit --verbose

echo "Stress testing..."
phpunit --repeat 100

if [[ $(phpenv version-name) =~ 5.[345] ]] ; then
    echo "Testing pux extension..."
    cd ext
    extunit --phpunit --verbose MuxTest.php || exit 1
    extunit --phpunit --verbose ControllerTest.php || exit 1

    echo "Stress testing on extension..."
    extunit --phpunit --repeat 100 MuxTest.php || exit 1
    extunit --phpunit --repeat 100 ControllerTest.php || exit 1
    cd ..
fi
