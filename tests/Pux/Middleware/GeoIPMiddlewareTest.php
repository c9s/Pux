<?php
use Pux\Middleware\GeoIPMiddleware;
use Pux\Testing\Utils;
use Geocoder\Geocoder;
use Geocoder\Provider\FreeGeoIp;
use Geocoder\Provider\GeoIP;
use Ivory\HttpAdapter\CurlHttpAdapter;
use Ivory\HttpAdapter\FileGetContentsHttpAdapter;


class GeoIPMiddlewareTest extends PHPUnit_Framework_TestCase
{
    public function testGeoIPMiddleware()
    {
        $testing = $this;
        $app = function($env, $res) use ($testing) {
            $testing->assertEquals('US', $env['geoip.country_code']);
            return $res;
        };

        // $adapter = new CurlHttpAdapter([ 'CURLOPT_CONNECTTIMEOUT' => 10000 ]);
        $adapter = new FileGetContentsHttpAdapter();
        $geocoder = new FreeGeoIp($adapter);
        $middleware = new GeoIPMiddleware($app, $geocoder);
        $globals = Utils::createGlobals('GET', '/');
        $globals['_SERVER']['REMOTE_ADDR'] = '173.194.72.113';
        $middleware($globals, []);
    }
}

