<?php
use Pux\Mux;
use Pux\Executor;
use Pux\Controller;

class ParentController extends Controller {

    /**
     *
     * @Route("/update")
     * @Method("GET")
     */
    public function pageAction() {  }
}

class ChildController extends ParentController { 
    // we should override this action.
    public function pageAction() {  }

    public function subpageAction() {  }

}


class ControllerAnnotationTest extends PHPUnit_Framework_TestCase
{

    public function testAnnotationForGetActionMethods()
    {
        $parent = new ParentController;
        ok($parent);
        ok( $map = $parent->getActionMethods() );
        ok( is_array($map), 'map is an array' );
        ok( isset($map[0]), 'one path' );
        is( 1, count($map), 'count of map' );
        is( 'pageAction', $map[0][0], 'pageAction');
        is([ 'Route' => '/update', 'Method' => 'GET',
            'class' => 'ParentController',
            'is_parent' => true,
        ], $map[0][1] );
    }


    public function testInheritedActions() 
    {
        $con = new ChildController;
        ok($con);
        ok( $map = $con->getActionMethods() );
        ok( is_array($map), 'map is an array' );

        is( 3, count($map), 'count of map should contain parent and child methods' );

        ok( is_array($map[0]), 'first path' );
        ok( is_array($map[1]), 'second path' );
        ok( is_array($map[2]), 'third path' );
        
        /*
        is( 'pageAction', $map[0][0], 'pageAction');
        is([ 'Route' => '/update', 'Method' => 'GET' ], $map[0][1] );
         */
    }



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

