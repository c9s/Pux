<?php
use Pux\Dispatcher\PathDispatcher;
use Pux\Mux;
use Pux\RouteExecutor;
use Pux\Controller\ExpandableController;

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
        $con = new ProductResource2Controller;
        $routes = $con->getActionRoutes();
        ok($routes);

        $methods = $con->parseActionMethods();
        $productMux = $con->expand();  // there is a sorting bug (fixed), this tests it.
        ok($productMux);

        $root = new Mux;
        ok($root);
        $root->mount('/product', $con->expand() );

        $dispatcher = new PathDispatcher($root);

        $_SERVER['REQUEST_METHOD'] = 'GET';
        ok( $dispatcher->dispatch('/product/10') == $root->dispatch('/product/10') );

        $_SERVER['REQUEST_METHOD'] = 'DELETE';
        ok( $dispatcher->dispatch('/product/10') == $root->dispatch('/product/10') );

        $_SERVER['REQUEST_METHOD'] = 'POST';
        ok( $dispatcher->dispatch('/product') == $root->dispatch('/product') ); // create

        $_SERVER['REQUEST_METHOD'] = 'POST';
        ok( $dispatcher->dispatch('/product/10') == $root->dispatch('/product/10') ); // update
    }
}

