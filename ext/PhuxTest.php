<?php
require 'PHPUnit/Framework/ExtensionTestCase.php';
require 'PHPUnit/TestMore.php';
require '../src/Phux/RouteCompiler.php';
require '../src/Phux/Mux.php';

class PhuxTest extends PHPUnit_Framework_ExtensionTestCase
{

    public $repeat = 10;

    public function getExtensionName()
    {
        return 'phux';
    }

    public function getFunctions()
    {
        return array(
            'phux_match',
        );
    }

    public function testMethodDispatch() {
        $routes = array(
            array(
                true, '#^ /product /(?P<id>[^/]+?) $#xs',
                array( 'ProductController', 'itemAction' ),
                array(
                    'require' => array ( 'id' => '\\d+',),
                    'default' => array ( 'id' => '1',),
                    'variables' => array ( 'id',),
                    'regex' => '    /product /(?P<id>[^/]+?) ',
                    'compiled' => '#^ /product /(?P<id>[^/]+?) $#xs',
                    'pattern' => '/product/:id',
                    'method'  => 2,  // POST method
                ),
            ),
        );
        $_SERVER['REQUEST_METHOD'] = "POST";

        $r = phux_match($routes, '/product/10');
        ok($r, "Found route");
        ok( $r[3] , "Got route options" );
        ok( $r[3]['vars'] , "Got route vars" );
        ok( $r[3]['vars']['id'] , "Got id" );
        $this->assertSame([ 'ProductController', 'itemAction' ] , $r[2], 'Same callback');
    }

    public function testPcreDispatch() {
        $routes = array(
            array(
                true, '#^ /product /(?P<id>[^/]+?) $#xs',
                array( 'ProductController', 'itemAction' ),
                array(
                    'require' => array ( 'id' => '\\d+',),
                    'default' => array ( 'id' => '1',),
                    'variables' => array ( 'id',),
                    'regex' => '    /product /(?P<id>[^/]+?) ',
                    'compiled' => '#^ /product /(?P<id>[^/]+?) $#xs',
                    'pattern' => '/product/:id',
                ),
            ),
        );
        $r = phux_match($routes, '/product/10');
        ok($r, "Found route");
        ok( $r[3] , "Got route options" );
        ok( $r[3]['vars'] , "Got route vars" );
        ok( $r[3]['vars']['id'] , "Got id" );
        $this->assertSame([ 'ProductController', 'itemAction' ] , $r[2], 'Same callback');
    }

    public function testStringDispatch() {
        $routes = array(
            array( false, '/product/item', ['ProductController', 'itemAction'], [] ),
            array( false, '/product', ['ProductController', 'listAction'], [] ),
        );
        $r = phux_match($routes, '/product');
        ok($r, "Found route");

        $r = phux_match($routes, '/product/item');
        ok($r, "Found route");
    }

    public function testMuxClass() {
        ok( class_exists('Phux\\MuxNew') );
    }


    public function testMuxStaticIdGenerator() {
        $id = \Phux\MuxNew::generate_id();
        ok($id, "got mux ID $id");
        ok(is_numeric($id), "got mux ID $id" );
        ok(is_integer($id), "got mux ID $id");
    }

    public function testMuxGetId() {
        $mux = new \Phux\MuxNew;
        $id = $mux->getId();
        ok( is_integer($id) );
        is( $id , $mux->getId() );
    }

    public function testMuxRouteFound() {
        $mux = new \Phux\MuxNew;
        ok($mux);
        $mux->add('/product', [ 'ProductController','listAction' ]);
        $route = $mux->matchRoute("/product");
        ok($route);
    }


    public function testMuxAddSimpleRoute() {
        $mux = new \Phux\MuxNew;
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

    public function testMuxAddPCRERoute() {
        $mux = new \Phux\MuxNew;
        ok($mux);
        $mux->add('/product/:id', [ 'ProductController','itemAction' ]);
        $mux->add('/product', [ 'ProductController','listAction' ]);

        $routes = $mux->getRoutes();
        ok($routes);
        count_ok(2, $routes);
        is( 2, $mux->length() );
    }

    public function testMuxExport() {
        $mux = new \Phux\MuxNew;
        ok($mux);
        $mux->add('/product/:id', [ 'ProductController','itemAction' ]);
        $mux->add('/product', [ 'ProductController','listAction' ]);
        $code = $mux->export();
        ok($code);
        eval('$newMux = ' . $code . ';');
        ok($newMux);

        $routes = $newMux->getRoutes();
        ok($routes); 
        count_ok( 2, $routes );
        is( 2,  $newMux->length() ); 
    }

    public function testMuxDispatch() {
        $mux = new \Phux\MuxNew;
        ok($mux);
        $mux->add('/product/:id', [ 'ProductController','itemAction' ]);
        $mux->add('/product', [ 'ProductController','listAction' ]);
        $route = $mux->dispatch('/product');
        ok($route);
        ok($route[0] == false);
        ok( is_string($route[1]) );
        is( "/product", $route[1] );
    }



    public function testMuxCompile() {
        return;
        $mux = new \Phux\MuxNew;
        ok($mux);
        $mux->add('/product/:id', [ 'ProductController','itemAction' ]);
        $mux->add('/product', [ 'ProductController','listAction' ]);
        $mux->compile("_cache.php");
    }


}

