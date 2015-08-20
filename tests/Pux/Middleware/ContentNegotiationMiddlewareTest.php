<?php
use Pux\Middleware\ContentNegotiationMiddleware;
use Pux\Testing\Utils;
use Negotiation\Negotiator;

class ContentNegotiationMiddlewareTest extends PHPUnit_Framework_TestCase
{
    public function testContentNegotiationMiddleware()
    {
        $testing = $this;
        $app = function(array $environment, array $response) use ($testing) {
            $testing->assertEquals('text/html', $environment['request.best_format']->getValue());
        };

        $globals = Utils::createGlobals('GET', '/');
        $globals['_SERVER']['HTTP_ACCEPT'] = 'text/html, application/xhtml+xml, application/xml;q=0.9, */*;q=0.8';
        $globals['negotiation.priorities'] = array('text/html', 'application/json');
        $m = new ContentNegotiationMiddleware($app, new Negotiator);
        $response = $m->call($globals, []);
    }
}

