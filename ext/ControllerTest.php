<?php
require 'PHPUnit/Framework/ExtensionTestCase.php';
require 'PHPUnit/TestMore.php';
require '../src/Pux/PatternCompiler.php';
require '../src/Pux/MuxCompiler.php';
require '../src/Pux/Executor.php';
use Pux\Mux;
use Pux\Executor;
use Pux\Controller;

class ProductController extends Controller
{

    public function addAction() {

    }

    public function delAction() {

    }

}

class ControllerTest extends PHPUnit_Framework_ExtensionTestCase
{
    public function getExtensionName()
    {
        return 'pux';
    }

    public function getFunctions()
    {
        return array();
    }

    public function testExtensionLoaded() 
    {
        ok( extension_loaded('pux') );
    }

    public function testController() {
        $controller = new ProductController;
        ok($controller);

        $actions = $controller->getActionMethods();
        ok($actions);
        ok( is_array($actions), 'is array' );
        count_ok( 2, $actions);

        $paths = $controller->getActionPaths();
        ok($paths);
        count_ok(2, $paths);
        ok( is_array($paths[0]) );
        ok( is_array($paths[1]) );

        $mux = $controller->expand();
        ok($mux);

        ok( $routes = $mux->getRoutes() );
    }


}

