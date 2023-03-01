<?php
require '../vendor/autoload.php';
use Pux\Router;
use Pux\Mux;



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

class ProductController {
    public function listAction() {
        return 'product list';
    }
    public function itemAction($id) { 
        return "product $id";
    }
}

$router = new Router([ 'cache_file' => 'mux.php' ]);

if ( false === $router->load() ) {
    // dispatch /page to hello action
    $router->add('/product', ['ProductController','listAction']);
    $router->add('/product/:id', ['ProductController','itemAction'] , [
        'require' => [ ':id' => '\d+', ],
        'default' => [ ':id' => '1', ]
    ]);

    /* 
    * route ProductController:index to  /product
    * route ProductController:item to  /product/item
    * route ProductController:list to  /product/list
    * */
    // $router->mount('/product', 'ProductController');
    $router->save();
}
$r = $router->dispatch('/product/1');
var_dump( $r ); 
