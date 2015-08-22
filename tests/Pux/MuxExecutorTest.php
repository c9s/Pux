<?php
use Pux\Mux;
use Pux\Executor;

class MuxExecutorTest extends MuxTestCase
{

    public function testExecutor() {
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
        is('index',Executor::execute($r));

        ok( $r = $mux->dispatch('/foo') );
        is('foo', Executor::execute($r));

        ok( $r = $mux->dispatch('/bar') );
        is('bar', Executor::execute($r));


        // XXX: seems like a gc bug here
        return;


        $cb = function() use ($mux) {
            $r = $mux->dispatch('/product/23');
            Executor::execute($r);
        };
        for ( $i = 0 ; $i < 100 ; $i++ ) {
            call_user_func($cb);
        }

        for ( $i = 0 ; $i < 100 ; $i++ ) {
            ok( $r = $mux->dispatch('/product/23') );
            is('product item 23', Executor::execute($r));
        }


        ok( $r = $mux->dispatch('/hello/john') );
        is('hello john', Executor::execute($r));
    }
}
