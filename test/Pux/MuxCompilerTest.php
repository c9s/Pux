<?php
use Pux\MuxCompiler;
use Pux\Mux;

class FooController
{
    public function index($name, $phone = null) { 
    }
}

class MuxCompilerTest extends MuxTestCase
{
    public function testMuxCompiler()
    {
        $mux = new Mux;
        $mux->add('/hello/:name', [ 'FooController', 'index' ]);
        $mux->compile("hello_mux.php");

        $mux2 = new Mux;
        $mux2->add('/bye/:name', [ 'FooController', 'index' ]);
        $mux2->compile("bye_mux.php");

        $compiler = new MuxCompiler;
        ok( $compiler->load("hello_mux.php") );
        ok( $compiler->load("bye_mux.php") );

        $compiler->compileReflectionParameters();

        ok( $compiler->compile("merged_mux.php") );


        path_ok( "merged_mux.php" );

        $mux = require "merged_mux.php";
        ok($mux);

        $routes = $mux->getRoutes();
        ok($routes);


        count_ok(2, $routes);

        ok( $mux->dispatch('/hello/John') );
        ok( $mux->dispatch('/bye/John') );

        unlink("merged_mux.php");
        unlink("hello_mux.php");
        unlink("bye_mux.php");
    }

    public function testMuxCompile() {
        $mux = new \Pux\Mux;
        ok($mux);
        $mux->add('/product/:id', [ 'ProductController','itemAction' ]);
        $mux->add('/product', [ 'ProductController','listAction' ]);
        $mux->add('/foo', [ 'ProductController','fooAction' ]);
        $mux->add('/bar', [ 'ProductController','barAction' ]);
        $mux->add('/', [ 'ProductController','indexAction' ]);

        $ret = $mux->compile("_test_mux.php");
        ok($ret, "compile successfully");

        $newMux = require "_test_mux.php";
        ok($newMux);

        ok( $r = $newMux->dispatch("/foo") );
        $this->assertNonPcreRoute($r, "/foo");

        ok( $r = $newMux->dispatch("/product") );
        $this->assertNonPcreRoute($r, "/product");

        ok( $r = $newMux->dispatch('/') );
        $this->assertNonPcreRoute($r, '/');

        ok( $r = $newMux->dispatch('/bar') );
        $this->assertNonPcreRoute($r, '/bar');

        ok( $r = $newMux->dispatch('/product/10') );
        $this->assertPcreRoute($r, '/product/:id');
    }


}

