<?php
use Pux\Mux;
class HelloController {
    public function helloAction() {
        return 'hello';
    }
}
$mux = require 'hello_mux.php';
$r = $mux->dispatch('/hello');
