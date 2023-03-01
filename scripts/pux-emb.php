<?php
$console = new Pux\Console;
try {
    $console->run( $argv );
} catch ( Exception $exception ) {
    echo $exception->getMessage(), "\n";
    exit(-1);
}
