<?php
use Pux\MuxBuilder\RESTfulMuxBuilder;
use Pux\Mux;
use Pux\Testing\Utils;

class RESTfulMuxBuilderTest extends PHPUnit_Framework_TestCase
{
    public function testRESTfulMuxBuilder()
    {
        $mux = new Mux;
        $builder = new RESTfulMuxBuilder($mux, [ 'prefix' => '/=' ]);
        $builder->addResource('product', new ProductResourceController);
        $mux = $builder->build();

        $this->assertInstanceOf('Pux\\Mux', $mux);


        // $env = Utils::createEnv('GET', '/=/product/10');

        $_SERVER['REQUEST_METHOD'] = 'GET';
        $this->assertNotNull($mux->dispatch('/=/product/10'));

        $_SERVER['REQUEST_METHOD'] = 'DELETE';
        $this->assertNotNull($mux->dispatch('/=/product/10') );
    }
}

