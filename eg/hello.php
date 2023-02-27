<?php
use Pux\Mux;
class HelloController {
    public function helloAction(): string {
        return 'hello';
    }
}
$mux = require __DIR__ . '/hello_mux.php';
$r = $mux->dispatch('/hello');
