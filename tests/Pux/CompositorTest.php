<?php
use Pux\Compositor;
use Pux\RouteRequest;
use Pux\Testing\Utils;

class CompositorTest extends PHPUnit_Framework_TestCase
{
    public function testCompositor()
    {
        $compositor = new Compositor;
        $compositor->enable('Pux\\Middleware\\TryCatchMiddleware', [ 'throw' => true ]);

        // TODO
        // $compositor->mount('/foo', function() {  });

        $compositor->app(function(array $environment, array $response) {
            $request = RouteRequest::createFromEnv($environment);
            $response[0] = 200;
            return $response;
        });
        $app = $compositor->wrap();

        $env = Utils::createGlobals('GET', '/foo');
        $res = [  ];
        $res = $app($env, $res);
        $this->assertNotEmpty($res);
        $this->assertEquals(200, $res[0]);
    }
}

