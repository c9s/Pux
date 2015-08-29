<?php
namespace Pux\Middleware;
use Pux\App;

/**
 *
 * Middleware is a PHPSGI application (a code reference) and a Server. Middleware
 * looks like an application when called from a server, and it in turn can call
 * other applications. It can be thought of a plugin to extend a PHPSGI
 * application.
 *
 *
 */
class Middleware implements App
{
    /**
     * @var Middleware
     */
    protected $next;




    public function __construct(callable $next)
    {
        $this->next = $next;
    }


    /**
     * The wrapping logic of the middleware
     *
     * @param array $environment
     * @param array $response
     * @return array Response array
     */
    public function call(array & $environment, array $response)
    {
        if ($n = $this->next) {
            return $n($environment, $response);
        }
        return $response;
    }

    public function __invoke(array & $environment, array $response)
    {
        return $this->call($environment, $response);
    }
    
}



