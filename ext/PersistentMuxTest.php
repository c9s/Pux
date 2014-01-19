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

    public function testStore() 
    {
        $mux = new Mux;
        $mux->id = 1;
        $mux->add('/', [ 'IndexController', 'index' ]);
        ok( pux_store_mux('phpunit1', $mux), 'mux stored' );
        // ok( pux_store_mux('phpunit1', $mux), 'mux updated' );
        // ok( pux_store_mux('phpunit1', $mux), 'mux updated again' );
        $m = pux_fetch_mux('phpunit1');
        ok($m ,'mux fetched');
        ok( is_object($m) , 'fetched mux is an object' );
        ok( $m instanceof Mux, 'fetched mux is an Mux object');
        // pux_delete_mux('phpunit1');
    }

    /**
     * @depends testStore
     */
    public function testFetch() 
    {
        $m = pux_fetch_mux('phpunit1');
        ok( $m, 'mux fetched' );
        ok( is_object($m) , 'mux should be an object' );
        ok( $m instanceof Mux, 'fetched object should be an instanceof Mux');
    }

    public function testPersistentDispatch() {
        // this one load mux from _test_mux.php
        $route = pux_persistent_dispatch('testing', '_test_mux.php', '/');
        $this->assertRoute($route);

        // this one use persistent mux store.
        $route = pux_persistent_dispatch('testing', '_test_mux.php', '/');
        $this->assertRoute($route);
    }

    public function assertRoute($route) {
        ok($route, 'route found');
        ok(is_array($route), 'route should be an array');
        ok(!empty($route), 'route should not be empty');
        ok(isset($route[0]), 'should found route pcre flag');
        ok(isset($route[1]), 'should found route pattern');
        ok(isset($route[2]), 'should found route callback');
    }
}
