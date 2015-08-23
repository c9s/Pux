<?php
namespace Pux\Middleware;
use Pux\Middleware;
use Negotiation\FormatNegotiator;
use Negotiation\Negotiator;

/**
 * HeadMiddleware: Auto delete response body in HEAD requests.
 *
 * This middleware deletes response body in HEAD requests.
 * 
 * Middleware Port of https://metacpan.org/pod/Plack::Middleware::Head
 */
class HeadMiddleware extends Middleware
{
    public function call(array & $environment, array $response)
    {
        if ($environment['REQUEST_METHOD'] == 'HEAD') {
            // FIXME: return callback to the app so that we can remove the
            // response body
            $response = parent::call($environment, $response);

            // Clean up the response body
            $response[2] = '';
            return $response;
        }
        return parent::call($environment, $response);
    }
}

