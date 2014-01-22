<?php
require 'PHPUnit/Framework/ExtensionTestCase.php';
require 'PHPUnit/TestMore.php';
require '../src/Pux/PatternCompiler.php';
require '../src/Pux/MuxCompiler.php';
require '../src/Pux/Executor.php';
use Pux\Mux;
use Pux\Executor;

class PersistentMuxTest extends PHPUnit_Framework_TestCase
{

    public function testStoreOnly() 
    {
        $mux = new Mux;
        $mux->add('/', [ 'IndexController', 'index' ]);
        $this->assertMux($mux);
        ok( pux_store_mux('phpunit1', $mux), 'mux stored' );
    }

    public function testStoreAndFetch() 
    {
        $mux = new Mux;
        $mux->id = 1;
        $mux->add('/', [ 'IndexController', 'index' ]);
        ok( pux_store_mux('phpunit1', $mux), 'mux stored' );
        // ok( pux_store_mux('phpunit1', $mux), 'mux updated' );
        // ok( pux_store_mux('phpunit1', $mux), 'mux updated again' );
        $m = pux_fetch_mux('phpunit1');
        $this->assertMux($m);
        count_ok(1, $m->getRoutes(), 'should get one route');
    }


    /**
     * @depends testStoreOnly
     */
    public function testFetch() 
    {
        $i = 10;
        while( $i--) {
            $m = pux_fetch_mux('phpunit1');
            $this->assertMux($m);
            count_ok(1, $m->getRoutes(), 'should get one route');
        }
    }

    public function testPersistentDispatch() {
        // this one load mux from _test_mux.php
        $route = pux_persistent_dispatch('testing', '_test_mux.php', '/');
        $this->assertRoute($route);
        // this one use persistent mux store.
        $route = pux_persistent_dispatch('testing', '_test_mux.php', '/');
        $this->assertRoute($route);
    }

    public function assertMux($m) 
    {
        ok( $m, 'found mux' );
        ok( is_object($m) , 'mux should be an object' );
        ok( $m instanceof Mux, 'fetched object should be an instanceof Mux');
        ok( is_array($m->getRoutes()), 'mux->getRoutes should return an array' );
        ok( is_array($m->routes), 'mux->routes public member should be an array');

        // Verify the route structure
        $routes = $m->getRoutes();
        foreach( $routes as $route ) {
            $this->assertRoute($route);
        }
    }

    public function assertRoute($route) {
        ok($route, 'route found');
        ok(is_array($route), 'route should be an array');
        ok(!empty($route), 'route should not be empty');
        count_ok(4, $route, 'correct route structure (4 items array)');

        ok(isset($route[0]), 'should found route pcre flag');
        ok(is_bool($route[0]), 'pcre flag should be bool');

        ok(isset($route[1]), 'should found route pattern');
        ok(is_string($route[1]), 'route pattern should be string');

        ok(isset($route[2]), 'should found route callback');
        ok(is_array($route[2]), 'route callback should be array');
        ok(!empty($route[2]), 'route callback should not be empty');

        ok(isset($route[3]), 'should found route options');
        ok(is_array($route[3]), 'route options should be array');
    }
}
