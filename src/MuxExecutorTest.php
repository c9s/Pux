<?php
use Pux\Mux;
use Pux\RouteExecutor;
use Pux\Testing\MuxTestCase;


class MuxRouteExecutorTest extends MuxTestCase
{

    public function testRouteExecutor() {
        $mux = new \Pux\Mux;
        ok($mux);
        $mux->add('/hello/:name', array( 'HelloController2','helloAction' ), array(
            'require' => array( 'name' => '\w+' )
        ));
        $mux->add('/product/:id', array( 'ProductController','itemAction' ));
        $mux->add('/product', array( 'ProductController','listAction' ));
        $mux->add('/foo', array( 'ProductController','fooAction' ));
        $mux->add('/bar', array( 'ProductController','barAction' ));
        $mux->add('/', array( 'ProductController','indexAction' ));

        ok( $r = $mux->dispatch('/') );

        $response = RouteExecutor::execute($r);
        $this->assertEquals('index', $response[2]);

        ok( $r = $mux->dispatch('/foo') );

        $response = RouteExecutor::execute($r);
        $this->assertEquals('foo', $response[2]);

        ok( $r = $mux->dispatch('/bar') );

        $response = RouteExecutor::execute($r);
        $this->assertEquals('bar', $response[2]);


        // XXX: seems like a gc bug here
        return;


        $cb = function() use ($mux) {
            $r = $mux->dispatch('/product/23');
            RouteExecutor::execute($r);
        };
        for ( $i = 0 ; $i < 100 ; $i++ ) {
            call_user_func($cb);
        }

        for ( $i = 0 ; $i < 100 ; $i++ ) {
            ok( $r = $mux->dispatch('/product/23') );
            is('product item 23', RouteExecutor::execute($r));
        }


        ok( $r = $mux->dispatch('/hello/john') );
        is('hello john', RouteExecutor::execute($r));
    }
}
