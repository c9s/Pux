<?php
require 'PHPUnit/Framework/ExtensionTestCase.php';
require 'PHPUnit/TestMore.php';
require '../src/Pux/PatternCompiler.php';
require '../src/Pux/MuxCompiler.php';
require '../src/Pux/Executor.php';
use Pux\Mux;
use Pux\Executor;

class HelloController
{

    public function helloAction($name) {
        return "hello $name";
    }

}

class ProductController
{
    public function indexAction() { return 'index'; }
    public function fooAction() { return 'foo'; }
    public function barAction() { return 'bar'; }

    public function itemAction($id)  { 
        return "product item $id";
    }
}

class MuxTest extends PHPUnit_Framework_ExtensionTestCase
{

    public $repeat = 10;

    public function getExtensionName()
    {
        return 'pux';
    }

    public function getFunctions()
    {
        return array(
            'pux_match',
            'pux_sort_routes',
        );
    }

    public function testExtensionLoaded() 
    {
        ok( extension_loaded('pux') );
    }

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

        $r = pux_match($routes, '/product/10');
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
        $r = pux_match($routes, '/product/10');
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
        $r = pux_match($routes, '/product');
        ok($r, "Found route");

        $r = pux_match($routes, '/product/item');
        ok($r, "Found route");
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

    public function testMuxAddPCRERoute() {
        $mux = new \Pux\Mux;
        ok($mux);
        $mux->add('/product/:id', [ 'ProductController','itemAction' ]);
        $mux->add('/product', [ 'ProductController','listAction' ]);

        $routes = $mux->getRoutes();
        ok($routes);
        count_ok(2, $routes);
        is( 2, $mux->length() );
    }

    public function testMuxGetMethod() {
        $mux = new \Pux\Mux;
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


    public function testSort() {
        $mux = new \Pux\Mux;
        ok($mux);
        $mux->add('/product/:id', [ 'ProductController','itemAction' ]);
        $mux->add('/product', [ 'ProductController','listAction' ]);
        $mux->sort();
    }

    public function testMuxExport() {
        $mux = new \Pux\Mux;
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
        $mux = new \Pux\Mux;
        ok($mux);
        $submux = new \Pux\Mux;
        $mux->mount( '/sub' , $submux);
    }

    public function testRouteWithDomainCondition() {
        $mux = new \Pux\Mux;
        $mux->expand = false;
        ok($mux, "got mux");
        $mux->add('/foo', [ 'HelloController','indexAction' ], [ 'domain' => 'test.dev' ]);
        $_SERVER['HTTP_HOST'] = 'test.dev';
        $route = $mux->dispatch('/foo');
        ok($route);

        $_SERVER['HTTP_HOST'] = 'github.com';
        $route = $mux->dispatch('/foo');
        ok(! $route);
    }

    public function testRouteWithId() {
        $mux = new \Pux\Mux;
        $mux->add('/hello/:name', [ 'HelloController','indexAction' ], [ 'id' => 'hello-name' ]);
        $mux->add('/foo', [ 'HelloController','indexAction' ], [ 'id' => 'foo' ]);

        ok($mux->routesById);
        ok($mux->routesById['foo']);
        $r = $mux->getRoute('foo');
        ok( $r ,'should return foo route');
        ok($mux->getRoute('hello-name'), 'should return hello-name route');
    }

    public function testMuxMountNoExpand() {
        $mux = new \Pux\Mux;
        $mux->expand = false;
        ok($mux, "got mux");
        $submux = new \Pux\Mux;
        $submux->any('/hello/:name', [ 'HelloController','indexAction' ]);
        $submux->any('/foo', [ 'HelloController','indexAction' ]);
        $mux->mount( '/sub' , $submux);
    }

    public function testMuxMountNoExpandAndDispatchToSubMux() 
    {
        $mux = new \Pux\Mux;
        $mux->expand = false;
        ok($mux);

        $submux = new \Pux\Mux;
        $submux->any('/hello/:name', [ 'HelloController','indexAction' ]);
        $submux->any('/foo', [ 'HelloController','indexAction' ]);
        $mux->mount( '/sub' , $submux);

        ok($submux, 'submux');
        ok($submux->getRoutes(), 'submux routes');

        ok($mux->getRoutes(), 'mux routes');

        $submux2 = $mux->getSubMux($submux->getId());
        ok($submux2, 'submux2');
        ok($submux2->getRoutes(), 'submux2 routes');

        $r = $mux->dispatch('/sub/hello/John');
        ok($r);
        $this->assertPcreRoute($r, '/hello/:name');

        $r = $mux->dispatch('/sub/foo');
        $this->assertNonPcreRoute($r, '/foo');
    }

    public function testSubmuxPcreRouteNotFound() {
        $mux = new \Pux\Mux;
        ok($mux);
        is(0, $mux->length());

        $submux = new \Pux\Mux;
        $submux->any('/hello/:name', [ 'HelloController','indexAction' ]);
        ok($submux);
        ok($routes = $submux->getRoutes());
        is(1, $submux->length());
        is(0, $mux->length());
        $mux->mount( '/sub' , $submux);

        ok( ! $mux->dispatch('/sub/foo') );
        ok( ! $mux->dispatch('/sub/hello') );
        ok( ! $mux->dispatch('/foo') );
    }

    public function testSubmuxPcreRouteFound() {
        $mux = new \Pux\Mux;
        ok($mux);
        is(0, $mux->length());

        $submux = new \Pux\Mux;
        $submux->any('/hello/:name', [ 'HelloController','indexAction' ]);
        ok($submux);
        ok($routes = $submux->getRoutes());
        is(1, $submux->length());
        is(0, $mux->length());
        $mux->mount( '/sub' , $submux);
        $r = $mux->dispatch('/sub/hello/John');
        ok($r);
        $this->assertPcreRoute($r, '/sub/hello/:name');
    }

    public function testMuxDispatch() {
        $mux = new \Pux\Mux;
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
        $mux = new \Pux\Mux;
        ok($mux);
        $mux->add('/product/:id', [ 'ProductController','itemAction' ]);
        $mux->add('/product', [ 'ProductController','listAction' ]);
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

    public function testExecutor() {
        $mux = new \Pux\Mux;
        ok($mux);
        $mux->add('/hello/:name', [ 'HelloController','helloAction' ], [
            'require' => [ 'name' => '\w+' ]
        ]);
        $mux->add('/product/:id', [ 'ProductController','itemAction' ]);
        $mux->add('/product', [ 'ProductController','listAction' ]);
        $mux->add('/foo', [ 'ProductController','fooAction' ]);
        $mux->add('/bar', [ 'ProductController','barAction' ]);
        $mux->add('/', [ 'ProductController','indexAction' ]);

        ok( $r = $mux->dispatch('/') );
        is('index',Executor::execute($r));

        ok( $r = $mux->dispatch('/foo') );
        is('foo', Executor::execute($r));

        ok( $r = $mux->dispatch('/bar') );
        is('bar', Executor::execute($r));

        $cb = function() use ($mux) {
            $r = $mux->dispatch('/product/23');
            Executor::execute($r);
        };
        for ( $i = 0 ; $i < 100 ; $i++ ) {
            call_user_func($cb);
        }

        for ( $i = 0 ; $i < 100 ; $i++ ) {
            ok( $r = $mux->dispatch('/product/23') );
            is('product item 23', Executor::execute($r));
        }


        ok( $r = $mux->dispatch('/hello/john') );
        is('hello john', Executor::execute($r));
    }

    public function testMuxCompile() {
        $mux = new \Pux\Mux;
        ok($mux);
        $mux->add('/product/:id', [ 'ProductController','itemAction' ]);
        $mux->add('/product', [ 'ProductController','listAction' ]);
        $mux->add('/foo', [ 'ProductController','fooAction' ]);
        $mux->add('/bar', [ 'ProductController','barAction' ]);
        $mux->add('/', [ 'ProductController','indexAction' ]);

        $ret = $mux->compile("_test_mux.php");
        ok($ret, "compile successfully");

        $newMux = require "_test_mux.php";
        ok($newMux);

        ok( $r = $newMux->dispatch("/foo") );
        $this->assertNonPcreRoute($r, "/foo");

        ok( $r = $newMux->dispatch("/product") );
        $this->assertNonPcreRoute($r, "/product");

        ok( $r = $newMux->dispatch('/') );
        $this->assertNonPcreRoute($r, '/');

        ok( $r = $newMux->dispatch('/bar') );
        $this->assertNonPcreRoute($r, '/bar');

        ok( $r = $newMux->dispatch('/product/10') );
        $this->assertPcreRoute($r, '/product/:id');
    }

    public function assertNonPcreRoute($route, $path = null) {
        $this->assertFalse($route[0], "Should be Non PCRE Route, Got: {$route[1]}");
        if ( $path ) {
            $this->assertSame($path, $route[1], "Should be $path, Got {$route[1]}");
        }
    }

    public function assertPcreRoute($route, $path = null) {
        $this->assertTrue($route[0], "Should be PCRE Route, Got: {$route[1]}" );
        if ( $path ) {
            $this->assertSame($path, $route[3]['pattern'], "Should be $path, Got {$route[1]}");
        }
    }

    public function testNullStaticRoutes() {
        $mux = Mux::__set_state(array(
            'id' => NULL,
            'routes' =>
            array (
                0 =>
                    array (
                        0 => false,
                        1 => '/hello',
                        2 =>
                        array (
                            0 => 'HelloController',
                            1 => 'helloAction',
                        ),
                        3 => array (),
                ),
            ),
            'submux' => array(),
            // 'staticRoutes' => array(),
            // 'expand' => true,
        ));
        ok($mux);
    }

}

