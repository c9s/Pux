<?php
// require '../src/Pux/Mux.php';
// var_dump( $_SERVER ); 
use Pux\Mux;
class HelloController {
    public function helloAction() {
        return 'hello';
    }
}

// $r = pux_persistent_dispatch('hello', 'hello_mux.php', '/hello');
// var_dump( $r ); 
// pux_store_mux('hello', new Mux);
// echo "stored\n";

// $mux = require 'hello_mux.php';
// $r = $mux->dispatch('/hello');
