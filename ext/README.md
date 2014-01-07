
# Install

    pear channel-discover pear.corneltek.com
    pear install -a corneltek/PHPUnit_TestMore
    pear install -a corneltek/PHPUnit_Framework_ExtensionTestCase
    pear install -a corneltek/ExtUnit

# Run TestCases

    extunit --phpunit PuxTest.php
    extunit --phpunit --repeat 100 PuxTest.php
    extunit --phpunit --debug PuxTest.php

