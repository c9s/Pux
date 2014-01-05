<?php
// require '../vendor/autoload.php';
require '../src/Phux/PatternCompiler.php';
require '../src/Phux/Mux.php';
require '../src/Phux/MuxCompiler.php';
use Phux\MuxCompiler;
use Phux\Mux;

class HelloController {
    public function helloAction() {
        return 'hello';
    }
}

$compiler = new MuxCompiler();
$compiler->load('hello_routes.php');
$compiler->compile('hello_mux.php');
