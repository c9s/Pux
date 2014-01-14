#!/bin/bash
if [[ $(phpenv version-name) =~ "5.[345]" ]] ; then
    pear channel-discover pear.corneltek.com
    pear install -a -f corneltek/PHPUnit_TestMore
    pear install -a -f corneltek/PHPUnit_Framework_ExtensionTestCase
    pear install -a -f corneltek/ExtUnit
    cd ext
    phpize && ./configure
    cd ..
fi
if [[ $(phpenv version-name) =~ "5.[34]" ]] ; then
    echo "extension=apc.so" >> ~/.phpenv/versions/$(phpenv version-name)/etc/php.ini
fi
if [[ $(phpenv version-name) =~ "5.5" ]] ; then
    pecl install APCu-beta
    echo "extension=apcc.so" >> ~/.phpenv/versions/$(phpenv version-name)/etc/php.ini
fi
if [[ $(phpenv version-name) =~ "hhvm" ]] ; then
    sudo apt-get install php5-dev
fi
