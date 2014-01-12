<?php
// require '../src/Pux/Mux.php';
// var_dump( $_SERVER ); 
use Pux\Mux;
class HelloController {
    public function helloAction() {
        return 'hello';
    }
}

$mux = require 'hello_mux.php';
$r = $mux->dispatch('/hello');
