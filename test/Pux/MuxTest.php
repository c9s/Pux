<?php
use Pux\Mux;
use Pux\Executor;

class MuxTest extends MuxTestCase
{
    public function testMuxRouteRouteDefine() {
        $mux = new Mux;
        $mux->add('/', ['IndexController', 'indexAction']);
    }

    public function testMuxRoutePCRERouteDefine() {
        $mux = new Mux;
        $mux->add('/hello/:name', ['IndexController', 'indexAction']);
    }

    public function testMuxId() {
        $mux = new Mux;
        $mux->add('/hello/:name', ['IndexController', 'indexAction']);
        ok($mux->getId());
    }

    public function testMuxClass() {
        ok( class_exists('Pux\\Mux') );
    }


    public function testMuxStaticIdGenerator() {
        $id = \Pux\Mux::generate_id();
        ok($id, "got mux ID $id");
        ok(is_numeric($id), "got mux ID $id" );
        ok(is_integer($id), "got mux ID $id");
    }

    public function testMuxGetId() 
    {
        $mux = new \Pux\Mux;
        $id = $mux->getId();
        ok( is_integer($id) );
        is( $id , $mux->getId() );
    }

    public function testMuxStringMatchRouteFound() 
    {
        $mux = new \Pux\Mux;
        ok($mux);
        $mux->add('/product', [ 'ProductController','listAction' ]);
        $route = $mux->match("/product");
        ok($route);
    }

    public function testMuxPcreMatchRouteFound() 
    {
        $mux = new \Pux\Mux;
        ok($mux);
        $mux->add('/product/:id', [ 'ProductController','itemAction' ]);
        $route = $mux->match("/product/30");
        ok($route);
    }


    public function testRouteGetterAndSetter() {
        $mux = new \Pux\Mux;
        ok($mux);
        $mux->add('/product', [ 'ProductController','listAction' ]);
        $routes = $mux->getRoutes();
        $mux->setRoutes($routes);
    }

    public function testMuxAddSimpleRoute() {
        $mux = new \Pux\Mux;
        ok($mux);
        $mux->add('/product', [ 'ProductController','listAction' ]);

        $routes = $mux->getRoutes();
        ok($routes, 'got routes');
        count_ok(1, $routes);
        is( 1, $mux->length() );

        $r = $routes[0];
        ok($r, 'found routes[0]');
        ok($r[0] == false, 'should be false');
        ok($r[1], 'route pattern'); 
        ok( is_string($r[1]) );
        ok($r[2], 'route callback'); 

        ok( is_array($r[3]) , 'route options'); 
        ok( empty($r[3]) , 'route options is empty'); 
    }

    public function testBasicRoutes() {
        $mux = new \Pux\Mux;
        ok($mux);
        $mux->add('/product/:id', [ 'ProductController','itemAction' ]);
        $mux->add('/product', [ 'ProductController','listAction' ]);
        return $mux;
    }

    /**
     * @depends testBasicRoutes
     */
    public function testMuxAddPCRERoute($mux) {
        $routes = $mux->getRoutes();
        ok($routes);
        count_ok(2, $routes);
        is( 2, $mux->length() );
    }

    /**
     * @depends testBasicRoutes
     */
    public function testSort($mux) {
        $mux->sort();
    }

    public function testRouteWithId() {
        $mux = new \Pux\Mux;
        $mux->add('/hello/:name', [ 'HelloController2','indexAction' ], [ 'id' => 'hello-name' ]);
        $mux->add('/foo', [ 'HelloController2','indexAction' ], [ 'id' => 'foo' ]);

        ok($mux->routesById);
        ok($mux->routesById['foo']);
        $r = $mux->getRoute('foo');
        ok( $r ,'should return foo route');
        ok($mux->getRoute('hello-name'), 'should return hello-name route');
    }


    /**
     * @depends testBasicRoutes
     */
    public function testMuxDispatch($mux) {
        $route = $mux->dispatch('/product');
        ok($route);
        ok($route[0] == false);
        ok( is_string($route[1]) );
        is( "/product", $route[1] );
    }

    /**
     * @depends testBasicRoutes
     */
    public function testMuxPcreDispatch($mux) {
        $route = $mux->dispatch('/product/10');
        ok($route,"found route");
        ok($route[0], "is a pcre route");
    }


    /*
    public function testEmptyPathDispatch() 
    {
        $mux = new \Pux\Mux;
        ok($mux);
        try {
            $mux->dispatch(null);
        } catch (Exception $e) {
            return;
        }
        $this->fail('Expecting error.');
    }
    */

}

