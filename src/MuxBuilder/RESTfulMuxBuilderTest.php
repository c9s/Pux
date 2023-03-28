<?php
use Pux\MuxBuilder\RESTfulMuxBuilder;
use Pux\Mux;
use Pux\Testing\Utils;

class RESTfulMuxBuilderTest extends \PHPUnit\Framework\TestCase
{
    public function testRESTfulMuxBuilder()
    {
        $mux = new Mux;
        $resTfulMuxBuilder = new RESTfulMuxBuilder($mux, [ 'prefix' => '/=' ]);
        $resTfulMuxBuilder->addResource('product', new ProductResourceController);

        $mux = $resTfulMuxBuilder->build();

        $this->assertInstanceOf(\Pux\Mux::class, $mux);


        // $env = Utils::createEnv('GET', '/=/product/10');

        $_SERVER['REQUEST_METHOD'] = 'GET';
        $this->assertNotNull($mux->dispatch('/=/product/10'));

        $_SERVER['REQUEST_METHOD'] = 'DELETE';
        $this->assertNotNull($mux->dispatch('/=/product/10') );
    }
}

