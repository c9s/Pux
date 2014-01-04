<?php
require '../vendor/autoload.php';
use Phux\Router;
use Phux\Mux;

/*
    return [
        '/' => [
            [ '{pattern}', {callback}, {options} ],
            [ '{pattern}', {callback}, {options} ],
            [ '{pattern}', {callback}, {options} ],
            [ '{pattern}', {callback}, {options} ],
            [ '{pattern}', {callback}, {options} ],
            [ '{pattern}', {callback}, {options} ],

            // dispatch this pattern to submux1
            [ '{pattern}', 'submux1', {options} ],
        ],
        'submux1' => [
            // pattern for sub path.
            [ '{pattern}', {callback}, {options} ],

        ],
    ];
 */

class HelloController {
    public function helloAction() {
        return 'hello';
    }
}

$router = new Router([ 'cache_file' => '_hello_mux.php' ]);

if ( false === $router->load() ) {
    // dispatch /page to hello action
    $router->add('/hello', ['HelloController','helloAction']);
    $router->save();
}
$r = $router->dispatch('/hello');
