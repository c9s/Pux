<?php
use Pux\Mux;
use Pux\Executor;

class MuxMountTest extends MuxTestCase
{

    public function testMuxMountEmpty() {
        $mux = new \Pux\Mux;
        ok($mux);
        $submux = new \Pux\Mux;
        $mux->mount( '/sub' , $submux);
    }

    public function testConstruct() {
        $submux = new \Pux\Mux;
        $submux->any('/hello/:name', [ 'HelloController2','indexAction' ]);
        ok($submux);
    }

    public function testLength() {
        $submux = new \Pux\Mux;
        $submux->any('/hello/:name', [ 'HelloController2','indexAction' ]);
        ok($submux);
        is(1, $submux->length());
    }

    public function testGetRoutes() {
        $submux = new \Pux\Mux;
        $submux->any('/hello/:name', [ 'HelloController2','indexAction' ]);
        ok($submux);

        ok($routes = $submux->getRoutes() );
        ok(is_array($routes));
        count_ok(1, $routes);
    }

    public function testMountEmptyRoutes()
    {
        $mux = new \Pux\Mux;
        ok($mux);
        is(0, $mux->length());
        $submux = new \Pux\Mux;
        ok($submux);
        $mux->mount( '/sub' , $submux);
    }

    public function testMountPcreRoutes() {
        $mux = new \Pux\Mux;
        $submux = new \Pux\Mux;
        ok($submux);
        $submux->any('/hello/:name', [ 'HelloController2','indexAction' ]);
        $mux->mount( '/sub' , $submux);
    }

    public function testMountStrRoutes() {
        $mux = new \Pux\Mux;
        $submux = new \Pux\Mux;
        ok($submux);
        $submux->any('/hello/str', [ 'HelloController2','indexAction' ]);
        $mux->mount( '/sub' , $submux);
    }


    public function testSubmuxPcreRouteNotFound() {
        $mux = new \Pux\Mux;
        ok($mux);
        is(0, $mux->length());

        $submux = new \Pux\Mux;
        $submux->any('/hello/:name', [ 'HelloController2','indexAction' ]);
        $submux->any('/hello/test', [ 'HelloController2','indexAction' ]);

        ok($submux);
        ok($routes = $submux->getRoutes());
        is(2, $submux->length());
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
        $submux->any('/hello/:name', [ 'HelloController2','indexAction' ]);
        ok($submux);
        ok($routes = $submux->getRoutes());
        is(1, $submux->length());
        is(0, $mux->length());
        $mux->mount( '/sub' , $submux);
        $r = $mux->dispatch('/sub/hello/John');
        ok($r);
        $this->assertPcreRoute($r, '/sub/hello/:name');
    }

}
