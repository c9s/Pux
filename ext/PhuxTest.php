<?php
require 'PHPUnit/Framework/ExtensionTestCase.php';
require 'PHPUnit/TestMore.php';
require '../src/Phux/PatternCompiler.php';

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
            'phux_sort_routes',
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
        ok( class_exists('Phux\\Mux') );
    }


    public function testMuxStaticIdGenerator() {
        $id = \Phux\Mux::generate_id();
        ok($id, "got mux ID $id");
        ok(is_numeric($id), "got mux ID $id" );
        ok(is_integer($id), "got mux ID $id");
    }

    public function testMuxGetId() {
        $mux = new \Phux\Mux;
        $id = $mux->getId();
        ok( is_integer($id) );
        is( $id , $mux->getId() );
    }

    public function testMuxRouteFound() {
        $mux = new \Phux\Mux;
        ok($mux);
        $mux->add('/product', [ 'ProductController','listAction' ]);
        $route = $mux->matchRoute("/product");
        ok($route);
    }


    public function testMuxAddSimpleRoute() {
        $mux = new \Phux\Mux;
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
        $mux = new \Phux\Mux;
        ok($mux);
        $mux->add('/product/:id', [ 'ProductController','itemAction' ]);
        $mux->add('/product', [ 'ProductController','listAction' ]);

        $routes = $mux->getRoutes();
        ok($routes);
        count_ok(2, $routes);
        is( 2, $mux->length() );
    }

    public function testMuxGetMethod() {
        $mux = new \Phux\Mux;
        ok($mux);
        $mux->get('/news', [ 'NewsController','listAction' ]);
        $mux->get('/news_item', [ 'NewsController','itemAction' ], []);

        $routes = $mux->getRoutes();
        ok($routes);
        count_ok(2, $routes);
        is( 2, $mux->length() );

        $_SERVER['REQUEST_METHOD'] = "GET";
        ok( $mux->dispatch('/news_item') );
        ok( $mux->dispatch('/news') );
    }


    public function testMuxExport() {
        $mux = new \Phux\Mux;
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

    public function testMuxMountEmpty() {
        $mux = new \Phux\Mux;
        ok($mux);
        $subMux = new \Phux\Mux;
        $mux->mount( '/sub' , $subMux);
    }

    public function testMuxMountNoExpand() {
        $mux = new \Phux\Mux;
        $mux->expandSubMux = false;
        ok($mux);
        $submux = new \Phux\Mux;
        $submux->add('/hello/:name', [ 'HelloController','indexAction' ]);
        $mux->mount( '/sub' , $submux);
    }

    public function testMuxMount() {
        $mux = new \Phux\Mux;
        ok($mux);
        $subMux = new \Phux\Mux;
        $subMux->add('/hello/:name', [ 'HelloController','indexAction' ]);
        $mux->mount( '/sub' , $subMux);
    }

    public function testMuxDispatch() {
        $mux = new \Phux\Mux;
        ok($mux);
        $mux->add('/product/:id', [ 'ProductController','itemAction' ]);
        $mux->add('/product', [ 'ProductController','listAction' ]);
        $route = $mux->dispatch('/product');
        ok($route);
        ok($route[0] == false);
        ok( is_string($route[1]) );
        is( "/product", $route[1] );
    }

    public function testMuxPcreDispatch() {
        $mux = new \Phux\Mux;
        ok($mux);
        $mux->add('/product/:id', [ 'ProductController','itemAction' ]);
        $mux->add('/product', [ 'ProductController','listAction' ]);
        $route = $mux->dispatch('/product/10');
        ok($route,"found route");
        ok($route[0], "is a pcre route");
    }

    public function testMuxCompile() {
        $mux = new \Phux\Mux;
        ok($mux);
        $mux->add('/product/:id', [ 'ProductController','itemAction' ]);
        $mux->add('/product', [ 'ProductController','listAction' ]);
        $ret = $mux->compile("_cache.php");
        ok($ret, "compile successfully");
    }


}

