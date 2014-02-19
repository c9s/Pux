<?php

class ControllerTest extends PHPUnit_Framework_TestCase
{

    public function test()
    {
        if (defined('HHVM_VERSION')) {
            echo "HHVM does not support Reflection to expand controller action methods";
            return;
        }

        $controller = new ExpandableProductController;
        ok($controller);

        ok( is_array( $controller->getActionMethods() ) );

        $mux = new Pux\Mux;
        $mux->mount('/product', $controller->expand());
        ok($mux);

        // var_dump( $mux->getRoutes() ); 

        ok( $mux->dispatch('/product/del') );
        ok( $mux->dispatch('/product/add') );
        ok( $mux->dispatch('/product/foo/bar') );
        ok( $mux->dispatch('/product/item') );
        ok( $mux->dispatch('/product') );

        
    }
}

