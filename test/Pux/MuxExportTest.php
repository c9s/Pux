<?php
use Pux\Mux;
use Pux\Executor;

class MuxExportTest extends MuxTestCase
{
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
    public function testExport($mux) {
        $code = $mux->export();
        ok($code);
        eval('$newMux = ' . $code . ';');
        ok($newMux);

        $routes = $newMux->getRoutes();
        ok($routes); 
        count_ok( 2, $routes );
        is( 2,  $newMux->length() ); 
    }
}
