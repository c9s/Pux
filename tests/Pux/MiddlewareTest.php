<?php
use Pux\Middleware;
use Pux\RouteRequest;
use Pux\Testing\Utils;

class TryCatchMiddleware extends Middleware
{
    public function call(array $environment, array $response)
    {
        try {
            if ($n = $this->next) {
                $response = $n($environment, $response);
            }
            // $res = $next($env, $res);
        } catch (Exception $e) {

        }
        return $response;
    }
}

class MiddlewareTest extends PHPUnit_Framework_TestCase
{

    public function testMiddleware()
    {
        $app = function(array $environment, array $response) {
            $request = RouteRequest::createFromGlobals($environment['_SERVER']['PATH_INFO'], $environment);
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




