<?php
// vim:fdm=marker:
use Pux\Mux;
use Pux\RouteExecutor;
use Pux\Controller\ExpandableController;
use Pux\Builder\ControllerRouteBuilder;
use Pux\Testing\MuxTestCase;


// /* CRUDProductController {{{*/
class CRUDProductController extends ExpandableController
{
    public function indexAction() { }

    public function addAction() { }

    public function delAction() { }

    public function jsonAction() {
        return $this->toJson(['foo' => 1]);
    }
}
/*}}}*/

class ControllerTest extends MuxTestCase
{
    public function testControllerConstructor() {
        $crudProductController = new CRUDProductController;
        ok($crudProductController);
        return $crudProductController;
    }


    /**
     * @depends testControllerConstructor
     */
    public function testGetActionMethods($controller)
    {
        $actions = ControllerRouteBuilder::parseActionMethods($controller);
        $this->assertNotEmpty($actions);
        $this->assertCount(4, $actions);
    }

    /**
     * @depends testControllerConstructor
     */
    public function testGetActionRoutes($controller) {
        $paths = ControllerRouteBuilder::build($controller);
        $this->assertNotEmpty($paths);
        $this->assertCount(4, $paths);
        ok(is_array($paths[0]));
        ok(is_array($paths[1]));
        ok(is_array($paths[2]));
    }

    /**
     * @depends testControllerConstructor
     */
    public function testExpand($controller)
    {
        $mux = $controller->expand();
        $this->assertInstanceOf(\Pux\Mux::class, $mux);
        $routes = $mux->getRoutes();
        $this->assertCount(4, $routes);
    }

    /**
     * @depends testControllerConstructor
     */
    public function testToJson($controller)
    {
        if (!extension_loaded('json')) {
            $this->markTestSkipped('json extension not found.');
        }

        $response = $controller->jsonAction();
        $this->assertNotEmpty(json_decode((string) $response[2], null, 512, JSON_THROW_ON_ERROR));
    }


    /**
     * @depends testControllerConstructor
     */
    public function testMountControllerObject($controller) {
        $mux = new Mux;
        $mux->mount( '/product' , $controller );
    }

    /**
     * @depends testControllerConstructor
     */
    public function testMount($controller) {
        $mainMux = new Mux;
        $mainMux->mount( '/product' , $controller->expand() );
        $this->assertNotEmpty($mainMux->getRoutes());

        $this->assertNonPcreRoute($mainMux->dispatch('/product'));
        $this->assertNonPcreRoute( $mainMux->dispatch('/product/add') );
        $this->assertNonPcreRoute( $mainMux->dispatch('/product/del') );
    }

    /**
     * @depends testControllerConstructor
     */
    public function testMountNoExpand($controller) {
        $mainMux = new Mux;
        $mainMux->mount('/product' , $controller);
        $mainMux->any( '/' , ['ProductController', 'indexAction'] );

        $this->assertNotEmpty($mainMux->getRoutes()); 
        $this->assertCount(5,  $mainMux->getRoutes(), 'route count should be 5');
        ok($r = $mainMux->dispatch('/product') , 'matched /product' ); // match indexAction
        $this->assertSame(['CRUDProductController', 'indexAction'], $r[2] );

        ok( $r = $mainMux->dispatch('/product/add') );
        $this->assertSame( ['CRUDProductController', 'addAction'], $r[2] );

        ok( $r = $mainMux->dispatch('/product/del') );
        $this->assertSame( ['CRUDProductController', 'delAction'], $r[2] );

        $this->assertNull($mainMux->dispatch('/foo'));
        $this->assertNull($mainMux->dispatch('/bar'));
    }


}

