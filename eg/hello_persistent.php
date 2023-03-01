<?php
// require '../src/Pux/Mux.php';
// var_dump( $_SERVER ); 
use Pux\Mux;
class HelloController {
    public function helloAction(): string {
        return 'hello';
    }
}

echo '<pre>';
echo "process ", getmypid(), "\n";

$r = pux_persistent_dispatch('hello', 'hello_mux.php', '/hello');
echo '</pre>';
var_dump( $r ); 
