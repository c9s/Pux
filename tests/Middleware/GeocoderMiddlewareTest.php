<?php
use Pux\Middleware\GeocoderMiddleware;
use Pux\Testing\Utils;
use Geocoder\Geocoder;
use Geocoder\Provider\FreeGeoIp;
use Geocoder\Provider\GeoIP;
use Ivory\HttpAdapter\CurlHttpAdapter;
use Ivory\HttpAdapter\FileGetContentsHttpAdapter;


class GeocoderMiddlewareTest extends PHPUnit_Framework_TestCase
{
    public function testGeocoderMiddleware()
    {
        try {

            $testing = $this;
            $app = function(array & $env, array $res) use ($testing) {
                $testing->assertEquals('US', $env['geoip.country_code']);
                return $res;
            };

        

            // $adapter = new CurlHttpAdapter([ 'CURLOPT_CONNECTTIMEOUT' => 10000 ]);
            $adapter = new FileGetContentsHttpAdapter();
            $geocoder = new FreeGeoIp($adapter);
            $middleware = new GeocoderMiddleware($app, $geocoder);
            $env = Utils::createEnv('GET', '/');
            $env['REMOTE_ADDR'] = '173.194.72.113';
            $middleware($env, []);

        } catch (\Ivory\HttpAdapter\HttpAdapterException $e) {
            // This allowes connection timeout failture

        }
    }
}

