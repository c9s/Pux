<?php
use Pux\Dispatcher\APCDispatcher;
use Pux\Mux;
use Pux\Executor;
use Pux\Controller;

class ProductResource2Controller extends Controller {


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

class APCDispatcherRESTfulTest extends PHPUnit_Framework_TestCase
{
    public function test()
    {
        if ( ! extension_loaded('apc') && ! extension_loaded('apcu') ) {
            // echo 'APC or APCu extension is required.';
            return;
        }

        $con = new ProductResource2Controller;
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

        $dispatcher = new APCDispatcher($root, array(
            'namespace' => 'tests',
            'expiry' => 10,
        ));

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

