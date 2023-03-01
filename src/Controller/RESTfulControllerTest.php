<?php

use Pux\Controller\ExpandableController;
use Pux\Builder\ControllerRouteBuilder;
use Pux\Mux;

class RESTfulControllerTest extends \PHPUnit\Framework\TestCase
{
    public function testRESTfulDispatch()
    {
        $productResourceController = new ProductResourceController;

        $routes = ControllerRouteBuilder::build($productResourceController);
        $this->assertNotEmpty($routes);
        $this->assertTrue(is_array($routes));

        $methods = ControllerRouteBuilder::parseActionMethods($productResourceController);
        $this->assertNotEmpty($methods);
        $productMux = $productResourceController->expand();  // there is a sorting bug (fixed), this tests it.
        $this->assertNotEmpty($productMux);

        $mux = new Mux;
        $mux->mount('/product', $productResourceController->expand());

        $_SERVER['REQUEST_METHOD'] = 'GET';
        $this->assertNotNull($mux->dispatch('/product/10'));

        $_SERVER['REQUEST_METHOD'] = 'DELETE';
        $this->assertNotNull( $mux->dispatch('/product/10') );

        $_SERVER['REQUEST_METHOD'] = 'POST';
        $this->assertNotNull( $mux->dispatch('/product') ); // create

        $_SERVER['REQUEST_METHOD'] = 'POST';
        $this->assertNotNull( $mux->dispatch('/product/10') ); // update
        
    }
}

