#!/bin/bash
if [[ $(phpenv version-name) =~ 5.[34] ]] ; then
    echo "extension=apc.so" >> ~/.phpenv/versions/$(phpenv version-name)/etc/php.ini
fi
# if [[ $(phpenv version-name) =~ 5.5 ]] ; then
#     # echo yes | pecl install -f  APCu-beta
#     # echo "extension=apcu.so" >> ~/.phpenv/versions/$(phpenv version-name)/etc/php.ini
# fi
