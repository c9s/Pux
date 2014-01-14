#!/bin/bash
echo "Basic testing..."
phpunit --debug

echo "Stress testing..."
phpunit --repeat 100

if [[ $(phpenv version-name) =~ "5.[345]" ]] ; then
    echo "Testing pux extension..."
    cd ext
    extunit --phpunit --debug MuxTest.php
    extunit --phpunit --debug ControllerTest.php

    echo "Stress testing..."
    extunit --phpunit --repeat 100 MuxTest.php
    extunit --phpunit --repeat 100 ControllerTest.php
    cd ..
fi
