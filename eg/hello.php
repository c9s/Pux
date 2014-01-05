<?php
// require '../src/Phux/Mux.php';
use Phux\Mux;

class HelloController {
    public function helloAction() {
        return 'hello';
    }
}

$mux = require 'hello_mux.php';
$r = $mux->dispatch('/hello');
