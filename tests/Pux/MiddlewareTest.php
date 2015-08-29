<?php
use Pux\Middleware;
use Pux\RouteRequest;
use Pux\Testing\Utils;
use Pux\Testing\MuxTestCase;
use Pux\Compositor;
use Pux\Middleware\TryCatchMiddleware;


class MiddlewareTest extends PHPUnit_Framework_TestCase
{


    public function testMiddleware()
    {
        $app = function(array $environment, array $response) {
            $request = RouteRequest::createFromEnv($environment);
            return $response;
        };

        $middleware = new TryCatchMiddleware($app);
        $middleware2 = new TryCatchMiddleware($middleware);

        // $request = RouteRequest::create('GET', '/path');
        $env = Utils::createGlobals('GET', '/foo');
        $response = $middleware2($env, [200, [], []]);
        $this->assertNotEmpty($response);
    }
}




