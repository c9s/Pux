<?php
require 'PHPUnit/Framework/ExtensionTestCase.php';
require 'PHPUnit/TestMore.php';
require '../src/Pux/PatternCompiler.php';
require '../src/Pux/MuxCompiler.php';
require '../src/Pux/Executor.php';
use Pux\Mux;
use Pux\Executor;

class HelloController
{

    public function helloAction($name) {
        return "hello $name";
    }

}

class ProductController
{
    public function indexAction() { return 'index'; }
    public function fooAction() { return 'foo'; }
    public function barAction() { return 'bar'; }

    public function itemAction($id)  { 
        return "product item $id";
    }
}

class PuxTest extends PHPUnit_Framework_ExtensionTestCase
{

    public $repeat = 10;

    public function getExtensionName()
    {
        return 'pux';
    }

    public function getFunctions()
    {
        return array(
            'pux_match',
            'pux_sort_routes',
        );
    }

    public function testExtensionLoaded() 
    {
        ok( extension_loaded('pux') );
    }


    public function testMuxRoutePCRERouteDefine() {
        $mux = new Mux;
        // $mux->add('/hello/:name', ['IndexController', 'indexAction']);
        $mux->add('/hello', ['IndexController', 'indexAction']);
        $mux->add('/foo', ['IndexController', 'indexAction']);
        $mux->add('/bar', ['IndexController', 'indexAction']);
    }

}

