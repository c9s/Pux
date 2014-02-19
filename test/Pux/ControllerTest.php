<?php
use Pux\Mux;
use Pux\Executor;
use Pux\Controller;

class CRUDProductController extends Controller
{
    public function indexAction() 
    {

    }

    public function addAction() {

    }

    public function delAction() {

    }

}

class ControllerTest extends PHPUnit_Framework_TestCase
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
        $mux->mount('/product', $controller->expand());
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

    public function testControllerMethods() {
        $controller = new CRUDProductController;
        ok($controller);

        $actions = $controller->getActionMethods();
        ok($actions);
        ok( is_array($actions), 'is array' );
        count_ok( 3, $actions);

        $paths = $controller->getActionRoutes();
        return;
        ok($paths);
        count_ok(3, $paths);
        ok( is_array($paths[0]) );
        ok( is_array($paths[1]) );
        ok( is_array($paths[2]) );

        $mux = $controller->expand();
        ok($mux);

        ok( $routes = $mux->getRoutes() );
        count_ok( 3, $routes );

        ok( $controller->toJson(array('foo' => 1) ) );

        $mainMux = new Mux;
        $mainMux->mount( '/product' , $controller->expand() );
        ok( $mainMux->getRoutes() ); 
        ok( $mainMux->dispatch('/product') );
        ok( $mainMux->dispatch('/product/add') );
        ok( $mainMux->dispatch('/product/del') );

        $mainMux = new Mux;
        $mainMux->expand = false;
        $mainMux->mount( '/product' , $controller->expand() );
        ok( $mainMux->getRoutes() ); 
        ok( $r = $mainMux->dispatch('/product') );
        ok( $r = $mainMux->dispatch('/product/add') );
        ok( $r = $mainMux->dispatch('/product/del') );
    }


}

