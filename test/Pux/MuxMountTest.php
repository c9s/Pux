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
        $submux->any('/hello/:name', array( 'HelloController2','indexAction' ));
        ok($submux);
    }

    public function testLength() {
        $submux = new \Pux\Mux;
        $submux->any('/hello/:name', array( 'HelloController2','indexAction' ));
        ok($submux);
        is(1, $submux->length());
    }

    public function testGetRoutes() {
        $submux = new \Pux\Mux;
        $submux->any('/hello/:name', array( 'HelloController2','indexAction' ));
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
        $submux->any('/hello/:name', array( 'HelloController2','indexAction' ));
        $mux->mount( '/sub' , $submux);
    }

    public function testMountStrRoutes() {
        $mux = new \Pux\Mux;
        $submux = new \Pux\Mux;
        ok($submux);
        $submux->any('/hello/str', array( 'HelloController2','indexAction' ));
        $mux->mount( '/sub' , $submux);
    }


    public function testSubmuxPcreRouteNotFound() {
        $mux = new \Pux\Mux;
        ok($mux);
        is(0, $mux->length());

        $submux = new \Pux\Mux;
        $submux->any('/hello/:name', array( 'HelloController2','indexAction' ));
        $submux->any('/hello/test', array( 'HelloController2','indexAction' ));

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
        $submux->any('/hello/:name', array( 'HelloController2','indexAction' ));
        ok($submux);
        ok($routes = $submux->getRoutes());
        is(1, $submux->length());
        is(0, $mux->length());
        $mux->mount( '/sub' , $submux);
        $r = $mux->dispatch('/sub/hello/John');
        ok($r);
        $this->assertPcreRoute($r, '/sub/hello/:name');
    }

    public function testSubmuxMergeOptions() {
        $mux = new \Pux\Mux;
        ok($mux);
        is(0, $mux->length());

        $submux = new \Pux\Mux;
        $submux->any('/hello/static', array( 'HelloController2', 'indexAction' ), array(
            'method' => Mux::getRequestMethodConstant('POST') // force POST
        ));
        $submux->any('/hello/:name', array( 'HelloController2', 'indexAction' ), array(
            'require' => array( 'name' => '[a-zA-Z]*' ) // allow a-z and A-Z
        ));
        ok($submux);
        ok($routes = $submux->getRoutes());
        is(2, $submux->length());
        is(0, $mux->length());
        $mux->mount( '/sub', $submux, array(
            'require' => array( 'name' => '[a-z]*' ), // only allow a-z
            'method' => Mux::getRequestMethodConstant('GET') // force GET
        ));
        is(2, $mux->length());

        $_SERVER['REQUEST_METHOD'] = 'GET';
        $r = $mux->dispatch('/sub/hello/John'); // uppercase J fails if it would still be a-z
        ok($r);
        $this->assertPcreRoute($r, '/sub/hello/:name');

        $_SERVER['REQUEST_METHOD'] = 'POST';
        $r = $mux->dispatch('/sub/hello/static'); // POST would fail if it would still be GET
        ok($r);
        $this->assertNonPcreRoute($r, '/sub/hello/static');
    }

}
