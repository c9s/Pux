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
        $compositor->enable(function($app) {
            return function(array $environment, array $response) use ($app) { 
                $environment['middleware.app'] = true;
                return $app($environment, $response);
            };
        });

        // TODO
        // $compositor->mount('/foo', function() {  });

        $compositor->app(function(array $environment, array $response) {
            $request = RouteRequest::createFromEnv($environment);


            // $mux = new Mux;

            if ($request->pathStartWith('/foo')) {

            }

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

