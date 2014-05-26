<?php
use Pux\Mux;
use Pux\Executor;

class MuxNoExpandMountTest extends MuxTestCase
{

    public function testMuxMountNoExpand() {
        $mux = new \Pux\Mux;
        $mux->expand = false;
        ok($mux, "got mux");
        $submux = new \Pux\Mux;
        $submux->any('/hello/:name', [ 'HelloController2','indexAction' ]);
        $submux->any('/foo', [ 'HelloController2','indexAction' ]);
        $mux->mount( '/sub' , $submux);
    }

    public function testMuxMountEmpty() {
        $mux = new \Pux\Mux;
        $mux->expand = false;
        ok($mux);
        $submux = new \Pux\Mux;
        $mux->mount( '/sub' , $submux);
    }

    public function testMuxMountNoExpandAndDispatchToSubMux() 
    {
        $mux = new \Pux\Mux;
        $mux->expand = false;
        ok($mux);

        $submux = new \Pux\Mux;
        $submux->any('/hello/:name', [ 'HelloController2','indexAction' ]);
        $submux->any('/foo', [ 'HelloController2','indexAction' ]);

        $mux->mount( '/sub' , $submux);
        ok($submux, 'submux');
        ok($submux->getRoutes(), 'submux routes');
        ok($mux->getRoutes(), 'mux routes');
        $submux2 = $mux->getSubMux($submux->getId());
        ok($submux2, 'submux2');
        ok($submux2->getRoutes(), 'submux2 routes');

        $r = $mux->dispatch('/sub/hello/John');
        ok($r);
        $this->assertPcreRoute($r, '/hello/:name');

        $r = $mux->dispatch('/sub/foo');
        $this->assertNonPcreRoute($r, '/foo');
    }

}
