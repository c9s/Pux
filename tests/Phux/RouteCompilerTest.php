<?php
use Phux\RouteCompiler;
use Phux\Mux;

class FooController
{
    public function index() {  }
}

class RouteCompilerTest extends PHPUnit_Framework_TestCase
{
    public function testRouteCompiler()
    {
        $mux = new Mux;
        $mux->add('/hello/:name', [ 'FooController', 'index' ]);
        $mux->compile("tests/hello_mux.php");

        $mux2 = new Mux;
        $mux2->add('/bye/:name', [ 'FooController', 'index' ]);
        $mux2->compile("tests/bye_mux.php");

        $compiler = new RouteCompiler;
        ok( $compiler->load("tests/hello_mux.php") );
        ok( $compiler->load("tests/bye_mux.php") );

        ok( $compiler->compile("tests/merged_mux.php") );

        path_ok( "tests/merged_mux.php" );

        $mux = require "tests/merged_mux.php";
        ok($mux);

        $routes = $mux->getRoutes();
        ok($routes);

        count_ok(2, $routes);

        ok( $mux->dispatch('/hello/John') );
        ok( $mux->dispatch('/bye/John') );

        unlink("tests/merged_mux.php");
        unlink("tests/hello_mux.php");
        unlink("tests/bye_mux.php");
    }
}

