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

    public function testStore() {
        $mux = new Mux;
        $mux->add('/', [ 'IndexController', 'index' ]);
        ok( pux_store_mux('phpunit1', $mux), 'mux stored' );
        // ok( pux_store_mux('phpunit1', $mux), 'mux updated' );
        // ok( pux_store_mux('phpunit1', $mux), 'mux updated again' );

        $m = pux_fetch_mux('phpunit1');

        // debug_zval_dump($m);
        ok($m ,'mux fetched');
        ok( is_object($m) , 'fetched mux is an object' );
        ok( $m instanceof Mux, 'fetched mux is an Mux object');

        // pux_delete_mux('phpunit1');
    }

    /**
     * @depends testStore
     */
    /*
    public function testFetch() {
        $m = pux_fetch_mux('phpunit1');
        ok( is_object($m) );
        ok( $m instanceof Mux);
    }

    public function testPersistentDispatch() {
        $route = pux_persistent_dispatch('testing', '_test_mux.php', '/');
        ok($route);
        ok(is_array($route));

        $route = pux_persistent_dispatch('testing', '_test_mux.php', '/');
        ok($route);
        ok(is_array($route));
    }
    */
}
