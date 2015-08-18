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

    public function testSubmuxPcreMount() {
        $mux = new \Pux\Mux;
        ok($mux);
        is(0, $mux->length());

        $submux = new \Pux\Mux;
        $submux->any('/hello/:name', array( 'HelloController2','indexAction' ));
        ok($submux);
        ok($routes = $submux->getRoutes());
        is(1, $submux->length());
        is(0, $mux->length());
        $mux->mount( '/sub/:country' , $submux);
        $r = $mux->dispatch('/sub/UK/hello/John');
        ok($r);
        $this->assertPcreRoute($r, '/sub/:country/hello/:name');
    }

    public function testSubmuxPcreMountStaticSub() {
        $mux = new \Pux\Mux;
        ok($mux);
        is(0, $mux->length());

        $submux = new \Pux\Mux;
        $submux->any('/hello/there', array('HelloController2', 'indexAction'));
        ok($submux);
        ok($routes = $submux->getRoutes());
        is(1, $submux->length());
        is(0, $mux->length());
        $mux->mount('/sub/:country', $submux);
        $r = $mux->dispatch('/sub/UK/hello/there');
        ok($r);
        $this->assertPcreRoute($r, '/sub/:country/hello/there');
    }

    public function testCallableSubMux() {
        $mux = new \Pux\Mux;
        $mux->mount('/test', function (Mux $submux) {
            $submux->any('/hello/static', array('HelloController2', 'indexAction'));
            $submux->any('/hello/:name', array('HelloController2', 'indexAction'));
        });

        ok($mux);

        ok($routes = $mux->getRoutes());
        ok(is_array($routes));
        count_ok(2, $routes);

        ok($routes[0][1] == '/test/hello/static');
        ok($routes[1][1] == '#^    /test/hello
    /(?P<name>[^/]+?)
$#xs');

        $r = $mux->dispatch('/test/hello/John');
        ok($r);
        $this->assertPcreRoute($r, '/test/hello/:name');

        $r = $mux->dispatch('/test/hello/static');
        $this->assertNonPcreRoute($r, '/test/hello/static');
    }

    public function testSubmuxMergeOptions()
    {
        $mux = new \Pux\Mux;
        ok($mux);
        is(0, $mux->length());

        $submux = new \Pux\Mux;
        $submux->any('/foo', array( 'HelloController2', 'indexAction' ), array(
            'default' => array('suffix' => 'csv'),
        ));
        $submux->any('/hello/:name', array( 'HelloController2', 'indexAction' ), array(
            'default' => array('suffix' => 'json')
        ));
        ok($submux);
        ok($routes = $submux->getRoutes());
        is(2, $submux->length());
        is(0, $mux->length());
        $mux->mount( '/sub', $submux, array(
            'default' => array('suffix' => 'xml', 'prefix' => 'v1'),
            'require' => array('suffix' => '[a-z]{3,4}')
        ));
        is(2, $mux->length());

        $r = $mux->dispatch('/sub/hello/John');
        ok($r);
        ok($r[3]['default']['suffix'] == 'json');
        ok($r[3]['default']['prefix'] == 'v1');
        ok($r[3]['require']['suffix'] == '[a-z]{3,4}');
        $this->assertPcreRoute($r, '/sub/hello/:name');

        $r = $mux->dispatch('/sub/foo');
        ok($r);
        ok($r[3]['default']['suffix'] == 'csv');
        ok($r[3]['default']['prefix'] == 'v1');
        ok($r[3]['require']['suffix'] == '[a-z]{3,4}');
        $this->assertNonPcreRoute($r, '/sub/foo');
    }
}
