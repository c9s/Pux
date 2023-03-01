<?php
// require '../vendor/autoload.php';
require '../src/Pux/PatternCompiler.php';
require '../src/Pux/Mux.php';
require '../src/Pux/MuxCompiler.php';
use Pux\MuxCompiler;
use Pux\Mux;

class HelloController {
    public function helloAction() {
        return 'hello';
    }
}

$compiler = new MuxCompiler();
$compiler->load('hello_routes.php');
$compiler->compile('hello_mux.php');
