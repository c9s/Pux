#!/bin/bash
phpize --clean
phpize && ./configure && make clean && make && make install
