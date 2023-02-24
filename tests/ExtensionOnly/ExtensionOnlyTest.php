<?php
use Pux\Mux;
use Pux\RouteExecutor;

class ExtensionOnlyMuxTest extends \PHPUnit\Framework\TestCase
{
    public function testFunctions()
    {
        $funcs = ['pux_match', 'pux_sort_routes'];
        if ( extension_loaded('pux') ) {
            foreach( $funcs as $func ) {
                ok( function_exists($func) );
            }
        }
    }

    public function setUp() {
        if ( ! extension_loaded('pux') ) {
            return $this->markTestSkipped('require pux extension');
        }
    }

    public function testPuxMethodDispatch() {

        $routes = [[true, '#^ /product /(?P<id>[^/]+?) $#xs', ['ProductController', 'itemAction'], ['require' => ['id' => '\\d+'], 'default' => ['id' => '1'], 'variables' => ['id'], 'regex' => '    /product /(?P<id>[^/]+?) ', 'compiled' => '#^ /product /(?P<id>[^/]+?) $#xs', 'pattern' => '/product/:id', 'method'  => 2]]];
        $_SERVER['REQUEST_METHOD'] = "POST";

        $r = pux_match($routes, '/product/10');
        ok($r, "Found route");
        ok( $r[3] , "Got route options" );
        ok( $r[3]['vars'] , "Got route vars" );
        ok( $r[3]['vars']['id'] , "Got id" );
        $this->assertSame(['ProductController', 'itemAction'] , $r[2], 'Same callback');
    }

    public function testPcreDispatch() {
        if ( ! extension_loaded('pux') ) {
            return $this->markTestSkipped('require pux extension');
        }

        $routes = [[true, '#^ /product /(?P<id>[^/]+?) $#xs', ['ProductController', 'itemAction'], ['require' => ['id' => '\\d+'], 'default' => ['id' => '1'], 'variables' => ['id'], 'regex' => '    /product /(?P<id>[^/]+?) ', 'compiled' => '#^ /product /(?P<id>[^/]+?) $#xs', 'pattern' => '/product/:id']]];
        $r = pux_match($routes, '/product/10');
        ok($r, "Found route");
        ok( $r[3] , "Got route options" );
        ok( $r[3]['vars'] , "Got route vars" );
        ok( $r[3]['vars']['id'] , "Got id" );
        $this->assertSame(['ProductController', 'itemAction'] , $r[2], 'Same callback');
    }

    public function testStringDispatch() {

        $routes = [[false, '/product/item', ['ProductController', 'itemAction'], []], [false, '/product', ['ProductController', 'listAction'], []]];
        $r = pux_match($routes, '/product');
        ok($r, "Found route");

        $r = pux_match($routes, '/product/item');
        ok($r, "Found route");
    }

    public function assertNonPcreRoute($route, $path = null) {
        $this->assertFalse($route[0], sprintf('Should be Non PCRE Route, Got: %s', $route[1]));
        if ( $path ) {
            $this->assertSame($path, $route[1], sprintf('Should be %s, Got %s', $path, $route[1]));
        }
    }

    public function assertPcreRoute($route, $path = null) {
        $this->assertTrue($route[0], sprintf('Should be PCRE Route, Got: %s', $route[1]) );
        if ( $path ) {
            $this->assertSame($path, $route[3]['pattern'], sprintf('Should be %s, Got %s', $path, $route[1]));
        }
    }

}

