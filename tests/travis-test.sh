#!/bin/bash
echo "Basic testing..."
phpunit --verbose

echo "Stress testing..."
phpunit --repeat 100

if [[ $(phpenv version-name) =~ 5.[345] ]] ; then
    echo "Testing pux extension..."
    cd ext
    extunit --phpunit --verbose MuxTest.php
    extunit --phpunit --verbose ControllerTest.php

    echo "Stress testing on extension..."
    extunit --phpunit --repeat 100 MuxTest.php
    extunit --phpunit --repeat 100 ControllerTest.php
    cd ..
fi
