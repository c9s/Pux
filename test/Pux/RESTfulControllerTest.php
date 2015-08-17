<?php
use Pux\Mux;
use Pux\Executor;
use Pux\Controller;

class ProductResourceController extends Controller { 


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

class RESTfulControllerTest extends PHPUnit_Framework_TestCase
{
    public function test()
    {
        $con = new ProductResourceController;
        $routes = $con->getActionRoutes();
        $this->assertNotEmpty($routes);
        $this->assertTrue(is_array($routes));

        $methods = $con->getActionMethods();
        $this->assertNotEmpty($methods);
        $productMux = $con->expand();  // there is a sorting bug (fixed), this tests it.
        $this->assertNotEmpty($productMux);

        $root = new Mux;
        $root->mount('/product', $con->expand());

        $_SERVER['REQUEST_METHOD'] = 'GET';
        $this->assertNotNull($root->dispatch('/product/10'));

        $_SERVER['REQUEST_METHOD'] = 'DELETE';
        $this->assertNotNull( $root->dispatch('/product/10') );

        $_SERVER['REQUEST_METHOD'] = 'POST';
        $this->assertNotNull( $root->dispatch('/product') ); // create

        $_SERVER['REQUEST_METHOD'] = 'POST';
        $this->assertNotNull( $root->dispatch('/product/10') ); // update
    }
}

