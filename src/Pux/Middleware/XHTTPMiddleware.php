<?php
namespace Pux\Middleware;
use Pux\Middleware;
use Negotiation\FormatNegotiator;
use Negotiation\Negotiator;

/**
 * XHTTPMiddleware: defines some features for extra HTTP mechanisms that are
 * not in the the HTTP protocol standards.
 *
 * Related issues:
 *
 * @see https://github.com/c9s/Pux/pull/90
 */
class XHTTPMiddleware extends Middleware
{
    public function call(array & $environment, array $response)
    {
        if (isset($environment['HTTP_X_HTTP_METHOD_OVERRIDE'])) {
            $environment['REQUEST_METHOD'] = $environment['HTTP_X_HTTP_METHOD_OVERRIDE'];
        }
        return parent::call($environment, $response);
    }
}

