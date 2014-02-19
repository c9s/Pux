<?php
use Pux\Mux;
use Pux\Executor;
use Pux\Controller;

class ControllerAnnotationTest extends PHPUnit_Framework_TestCase
{
    public function testAnnotations()
    {
        if (defined('HHVM_VERSION')) {
            echo "HHVM does not support Reflection to expand controller action methods";
            return;
        }

        $controller = new ExpandableProductController;
        ok($controller);

        ok( is_array( $controller->getActionMethods() ) );

        $mux = new Pux\Mux;

        // works fine
        // $submux = $controller->expand();
        // $mux->mount('/product', $submux );

        // gc scan bug
        $mux->mount('/product', $controller->expand() );
        ok($mux);

        $paths = array(
            '/product/delete' => 'DELETE',
            '/product/update' => 'PUT' ,
            '/product/add'    => 'POST' ,
            '/product/foo/bar' => null,
            '/product/item' => 'GET',
            '/product' => null,
        );

        foreach( $paths as $path => $method ) {
            if ( $method ) {
                $_SERVER['REQUEST_METHOD'] = $method;
            }
            ok( $mux->dispatch($path) , $path);
        }
    }


}

