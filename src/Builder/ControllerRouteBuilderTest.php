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
        $childController = new ChildController;
        $map = ControllerRouteBuilder::parseActionMethods($childController);
        $this->assertNotEmpty($map);
        $this->assertTrue(is_array($map));

        $this->assertTrue( isset($map['postAction']) );
        $this->assertTrue( isset($map['pageAction']) );
        $this->assertTrue( isset($map['subpageAction']) );

        $this->assertEquals([["Route" => "/post", "Method" => "POST"], ["class" => ChildController::class]], $map['postAction'] );

        $routeMap = ControllerRouteBuilder::build($childController);
        $this->assertCount(3, $routeMap);

        [$path, $method, $options] = $routeMap[0];
        is('/page', $path);
        is('pageAction', $method);
        is( ['method' => REQUEST_METHOD_GET] , $options);
    }


    public function testAnnotations()
    {
        if (defined('HHVM_VERSION')) {
            echo "HHVM does not support Reflection to expand controller action methods";
            return;
        }

        $expandableProductController = new \ExpandableProductController;
        $this->assertTrue(is_array( $map = ControllerRouteBuilder::parseActionMethods($expandableProductController) ) );

        $routes = ControllerRouteBuilder::build($expandableProductController);
        $this->assertNotEmpty($routes);

        $this->assertEquals('', $routes[0][0], 'the path');
        $this->assertEquals('indexAction', $routes[0][1], 'the mapping method');

        $mux = new Mux;

        // works fine
        // $submux = $controller->expand();
        // $mux->mount('/product', $submux );

        // gc scan bug
        $mux->mount('/product', $expandableProductController->expand() );

        $paths = ['/product/delete' => 'DELETE', '/product/update' => 'PUT', '/product/add'    => 'POST', '/product/foo/bar' => null, '/product/item' => 'GET', '/product' => null];
        foreach( $paths as $path => $method ) {
            if ($method !== null) {
                $_SERVER['REQUEST_METHOD'] = $method;
            } else {
                $_SERVER['REQUEST_METHOD'] = 'GET';
            }

            ok($mux->dispatch($path) , $path);
        }
    }
}

