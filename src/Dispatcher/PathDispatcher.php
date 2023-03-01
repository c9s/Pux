<?php
namespace Pux\Dispatcher;
use Pux\Mux;

/**
 * Dispath routes by path
 */
class PathDispatcher
{
    protected $options = [];

    protected $mux;

    protected $namespace = 'app';

    public $expiry = 0;

    public function __construct(Mux $mux, array $options = [])
    {
        $this->mux = $mux;
        $this->options = $options;
    }

    public function dispatchRequest(RouteRequest $routeRequest)
    {
        return $this->dispatch($routeRequest->getPath());
    }

    public function dispatch($path) 
    {
        if ($route = $this->mux->dispatch($path)) {
            return $route;
        }

        return false;
    }
}



