<?php

use Pux\Dispatcher\PathDispatcher;
use Pux\Mux;
use Pux\RouteExecutor;
use Pux\Controller\ExpandableController;
use Pux\Builder\ControllerRouteBuilder;

class ProductResource2Controller extends ExpandableController {


    /**
     * @Method("POST")
     * @Route("");
     */
    public function createAction() {

    }

    /**
     * @Route("/:id")
     * @Method("POST")
     */
    public function updateAction() {

    }

    /**
     * @Route("/:id")
     * @Method("GET")
     */
    public function getAction() {

    }

    /**
     * @Route("/:id");
     * @Method("DELETE")
     */
    public function deleteAction() {

    }

}

class DispatcherRESTfulTest extends \PHPUnit\Framework\TestCase
{
    public function test()
    {
        $productResource2Controller = new ProductResource2Controller;

        $routes = ControllerRouteBuilder::build($productResource2Controller);
        $this->assertNotEmpty($routes);

        ControllerRouteBuilder::parseActionMethods($productResource2Controller);
        $productMux = $productResource2Controller->expand();  // there is a sorting bug (fixed), this tests it.
        ok($productMux);

        $mux = new Mux;
        ok($mux);
        $mux->mount('/product', $productResource2Controller->expand() );

        $pathDispatcher = new PathDispatcher($mux);

        $_SERVER['REQUEST_METHOD'] = 'GET';
        ok( $pathDispatcher->dispatch('/product/10') == $mux->dispatch('/product/10') );

        $_SERVER['REQUEST_METHOD'] = 'DELETE';
        ok( $pathDispatcher->dispatch('/product/10') == $mux->dispatch('/product/10') );

        $_SERVER['REQUEST_METHOD'] = 'POST';
        ok( $pathDispatcher->dispatch('/product') == $mux->dispatch('/product') ); // create

        $_SERVER['REQUEST_METHOD'] = 'POST';
        ok( $pathDispatcher->dispatch('/product/10') == $mux->dispatch('/product/10') ); // update
    }
}

