<?php
namespace Pux;

class Middleware
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
    public function call(array $environment, array $response)
    {
        return $response;
    }

    public function __invoke(array $environment, array $response)
    {
        return $this->call($environment, $response);
    }
    
}



