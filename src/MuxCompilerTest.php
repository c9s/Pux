<?php
use Pux\MuxCompiler;
use Pux\Mux;
use Pux\Testing\MuxTestCase;


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
        $mux->add('/hello/:name', ['FooController', 'index']);
        $mux->compile("hello_mux.php");

        $mux2 = new Mux;
        $mux2->add('/bye/:name', ['FooController', 'index']);
        $mux2->compile("bye_mux.php");

        $muxCompiler = new MuxCompiler;
        ok( $muxCompiler->load("hello_mux.php") );
        ok( $muxCompiler->load("bye_mux.php") );

        $muxCompiler->compileReflectionParameters();

        $muxCompiler->compile("merged_mux.php");

        path_ok( "merged_mux.php" );

        $mux = require __DIR__ . "/merged_mux.php";
        $this->assertNotNull($mux);

        $routes = $mux->getRoutes();
        $this->assertNotNull($routes);

        $this->assertCount(2, $routes);

        $this->assertNotNull($mux->dispatch('/hello/John'));
        $this->assertNotNull( $mux->dispatch('/bye/John') );

        unlink("merged_mux.php");
        unlink("hello_mux.php");
        unlink("bye_mux.php");
    }

    public function testMuxCompile()
    {
        $mux = new Mux;
        $mux->add('/product/:id', ['ProductController', 'itemAction']);
        $mux->add('/product', ['ProductController', 'listAction']);
        $mux->add('/foo', ['ProductController', 'fooAction']);
        $mux->add('/bar', ['ProductController', 'barAction']);
        $mux->add('/', ['ProductController', 'indexAction']);

        $ret = $mux->compile("_test_mux.php");
        ok($ret, "compile successfully");

        $newMux = require __DIR__ . "/_test_mux.php";
        $this->assertNotNull($newMux);

        $this->assertNotNull( $r = $newMux->dispatch("/foo") );
        $this->assertNonPcreRoute($r, "/foo");

        $this->assertNotNull( $r = $newMux->dispatch("/product") );
        $this->assertNonPcreRoute($r, "/product");

        $this->assertNotNull( $r = $newMux->dispatch('/') );
        $this->assertNonPcreRoute($r, '/');

        $this->assertNotNull( $r = $newMux->dispatch('/bar') );
        $this->assertNonPcreRoute($r, '/bar');

        $this->assertNotNull( $r = $newMux->dispatch('/product/10') );
        $this->assertPcreRoute($r, '/product/:id');
    }


}

