<?php
// require '../vendor/autoload.php';
require __DIR__ . '/../src/Pux/PatternCompiler.php';
require __DIR__ . '/../src/Pux/Mux.php';
require __DIR__ . '/../src/Pux/MuxCompiler.php';
use Pux\MuxCompiler;
use Pux\Mux;

class HelloController {
    public function helloAction(): string {
        return 'hello';
    }
}

$compiler = new MuxCompiler();
$compiler->load('hello_routes.php');
$compiler->compile('hello_mux.php');
