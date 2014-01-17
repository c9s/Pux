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
        ok( pux_store_mux('phpunit1', $mux), 'mux updated' );
        $m = pux_fetch_mux('phpunit1');
        ok($m);
        ok( is_object($m) );
        ok( $m instanceof Mux);
    }



    /**
     * @depends testStore
     */
    public function testFetch() {
        $m = pux_fetch_mux('phpunit1');
        ok( is_object($m) );
        ok( $m instanceof Mux);
    }

    public function testPersistentDispatch() {
        pux_persistent_dispatch('testing', '_test_mux.php', '/');
    }
}
