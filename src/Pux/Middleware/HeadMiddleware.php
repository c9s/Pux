<?php
namespace Pux\Middleware;
use Pux\Middleware;
use Negotiation\FormatNegotiator;
use Negotiation\Negotiator;

class HeadMiddleware extends Middleware
{
    public function call(array & $environment, array $response)
    {
        if ($environment['REQUEST_METHOD'] == 'HEAD') {
            // FIXME: return callback to the app so that we can remove the
            // response body
            $response = parent::call($environment, $response);
        }
        return parent::call($environment, $response);
    }
}

