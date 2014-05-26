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
        ok($con);
        $routes = $con->getActionRoutes();
        ok($routes);

        $methods = $con->getActionMethods();
        ok($methods);
        $productMux = $con->expand();  // there is a sorting bug (fixed), this tests it.
        ok($productMux);

        $root = new Mux;
        ok($root);
        $root->mount('/product', $con->expand() );

        $_SERVER['REQUEST_METHOD'] = 'GET';
        ok( $root->dispatch('/product/10') );

        $_SERVER['REQUEST_METHOD'] = 'DELETE';
        ok( $root->dispatch('/product/10') );

        $_SERVER['REQUEST_METHOD'] = 'POST';
        ok( $root->dispatch('/product') ); // create

        $_SERVER['REQUEST_METHOD'] = 'POST';
        ok( $root->dispatch('/product/10') ); // update
    }
}

