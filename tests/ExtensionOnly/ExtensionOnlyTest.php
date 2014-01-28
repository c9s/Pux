<?php
use Pux\Mux;
use Pux\Executor;

class ExtensionOnlyMuxTest extends PHPUnit_Framework_TestCase
{
    public function testFunctions()
    {
        $funcs = array(
            'pux_match',
            'pux_sort_routes',
        );
        if ( extension_loaded('pux') ) {
            foreach( $funcs as $f ) {
                ok( function_exists($f) );
            }
        }
    }

    public function setUp() {
        if ( ! extension_loaded('pux') ) {
            return $this->markTestSkipped('require pux extension');
        }
    }

    public function testPuxMethodDispatch() {

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
        if ( ! extension_loaded('pux') ) {
            return $this->markTestSkipped('require pux extension');
        }
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

}

