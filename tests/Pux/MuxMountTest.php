<?php
use Pux\Mux;
use Pux\RouteExecutor;
use Pux\Testing\MuxTestCase;


class MuxMountTest extends MuxTestCase
{

    public function testMuxMountEmpty() {
        $mux = new Mux;
        $submux = new Mux;
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
        $this->assertPcreRoute($r, '/sub/hello/:name');
    }

    public function testSubmuxPcreMount()
    {
        $mux = new Mux;
        $this->assertEquals(0, $mux->length());

        $submux = new Mux;
        $submux->any('/hello/:name', array( 'HelloController2','indexAction' ));
        $submux->any('/goodbye', array( 'GoodbyeController2','indexAction' ));

        ok($routes = $submux->getRoutes());
        $this->assertEquals(2, $submux->length());

        $mux->mount( '/sub/:country' , $submux);

        $this->assertEquals(2, $mux->length());

        $route = $mux->dispatch('/sub/UK/goodbye');
        $this->assertNotNull($route);
        // $this->assertPcreRoute($r, '/hello/:name');
    }

    public function testSubmuxPcreMountStaticSub() {
        $mux = new Mux;
        $this->assertEquals(0, $mux->length());

        $submux = new \Pux\Mux;
        $submux->any('/hello/there', array('HelloController2', 'indexAction'));

        ok($routes = $submux->getRoutes());
        $this->assertEquals(1, $submux->length());
        $this->assertEquals(0, $mux->length());

        $mux->mount('/sub/:country', $submux);
        $r = $mux->dispatch('/sub/UK/hello/there');
        $this->assertPcreRoute($r, '/sub/:country/hello/there');
    }

    public function testCallableSubMux() {
        $mux = new \Pux\Mux;
        $mux->mount('/test', function (Mux $submux) {
            $submux->any('/hello/static', array('HelloController2', 'indexAction'));
            $submux->any('/hello/:name', array('HelloController2', 'indexAction'));
        });

        ok($routes = $mux->getRoutes());
        ok(is_array($routes));
        $this->assertCount(2, $routes);
        $this->assertEquals('/test/hello/static', $routes[0][1]);

        $r = $mux->dispatch('/test/hello/John');
        $this->assertNotEmpty($r);
        $this->assertPcreRoute($r, '/test/hello/:name');

        $r = $mux->dispatch('/test/hello/static');
        $this->assertNotEmpty($r);
        $this->assertNonPcreRoute($r, '/test/hello/static');
    }

    public function testSubmuxMergeOptions()
    {
        $mux = new Mux;
        $this->assertEquals(0, $mux->length());

        $submux = new Mux;
        $submux->any('/foo', array( 'FooController', 'indexAction' ), array(
            'default' => array(
                'suffix' => 'csv',
            ),
        ));
        $submux->any('/hello/:name', array( 'HelloController', 'indexAction' ), array(
            'default' => array(
                'suffix' => 'json',
                'prefix' => 'v1',
            ),
        ));
        $this->assertEquals(2, $submux->length());


        ok($routes = $submux->getRoutes());
        $this->assertEquals(2, $submux->length());
        $this->assertEquals(0, $mux->length());

        // XXX: we don't expand routes now, so we won't have the default
        //      options for the subroutes...
        /*
        $mux->mount('/sub', $submux, array(
            'default' => array('suffix' => 'xml', 'prefix' => 'v1'),
            'require' => array('suffix' => '[a-z]{3,4}')
        ));
        */
        $mux->mount('/sub', $submux);

        $this->assertEquals(2, $mux->length());

        $r = $mux->dispatch('/sub/hello/John');
        $this->assertNotNull($r);

        $this->assertEquals('json', $r[3]['default']['suffix']);
        $this->assertEquals('v1', $r[3]['default']['prefix']);
        $this->assertPcreRoute($r, '/sub/hello/:name');

        $r = $mux->dispatch('/sub/foo');
        $this->assertNotNull($r);
        // ok($r[3]['default']['suffix'] == 'csv');
        // ok($r[3]['require']['suffix'] == '[a-z]{3,4}');
        $this->assertNonPcreRoute($r, '/sub/foo');
    }
}
