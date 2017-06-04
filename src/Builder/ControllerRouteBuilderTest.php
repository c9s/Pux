<?php

namespace Pux\Controller;

use Pux\Mux;
use Pux\RouteExecutor;
use Pux\Builder\ControllerRouteBuilder;

class ParentController extends ExpandableController {

    /**
     *
     * @Route("/page")
     * @Method("GET")
     */
    public function pageAction() {  }


    /**
     * @Route("/post")
     * @Method("POST")
     */
    public function postAction() { }
}

class ChildController extends ParentController { 

    // we should override this action but use the parent annotations
    public function pageAction() {  }

    public function subpageAction() {  }
}


class ControllerRouteBuilderTest extends \PHPUnit\Framework\TestCase
{
    public function testAnnotationForGetActionMethods()
    {
        $con = new ChildController;
        $map = ControllerRouteBuilder::parseActionMethods($con);
        $this->assertNotEmpty($map);
        $this->assertTrue(is_array($map));

        $this->assertTrue( isset($map['postAction']) );
        $this->assertTrue( isset($map['pageAction']) );
        $this->assertTrue( isset($map['subpageAction']) );

        $this->assertEquals(array(
            array(
                "Route" => "/post",
                "Method" => "POST"
            ),
            array("class" => ChildController::class)
        ), $map['postAction'] );

        $routeMap = ControllerRouteBuilder::buildActionRoutes($con);
        $this->assertCount(3, $routeMap);

        list($path, $method, $options) = $routeMap[0];
        is('/page', $path);
        is('pageAction', $method);
        is( array( 'method' => REQUEST_METHOD_GET ) , $options);
    }


    public function testAnnotations()
    {
        if (defined('HHVM_VERSION')) {
            echo "HHVM does not support Reflection to expand controller action methods";
            return;
        }

        $controller = new \ExpandableProductController;
        $this->assertTrue(is_array( $map = ControllerRouteBuilder::parseActionMethods($controller) ) );

        $routes = ControllerRouteBuilder::buildActionRoutes($controller);
        $this->assertNotEmpty($routes);

        $this->assertEquals('', $routes[0][0], 'the path');
        $this->assertEquals('indexAction', $routes[0][1], 'the mapping method');

        $mux = new Mux;

        // works fine
        // $submux = $controller->expand();
        // $mux->mount('/product', $submux );

        // gc scan bug
        $mux->mount('/product', $controller->expand() );

        $paths = array(
            '/product/delete' => 'DELETE',
            '/product/update' => 'PUT' ,
            '/product/add'    => 'POST' ,
            '/product/foo/bar' => null,
            '/product/item' => 'GET',
            '/product' => null,
        );
        foreach( $paths as $path => $method ) {
            if ($method) {
                $_SERVER['REQUEST_METHOD'] = $method;
            } else {
                $_SERVER['REQUEST_METHOD'] = 'GET';
            }
            ok($mux->dispatch($path) , $path);
        }
    }
}

