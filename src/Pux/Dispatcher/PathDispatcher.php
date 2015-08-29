<?php
namespace Pux\Dispatcher;
use Pux\Mux;

/**
 * Dispath routes by path
 */
class PathDispatcher
{
    protected $options = array();

    protected $mux;

    protected $namespace = 'app';

    public $expiry = 0;

    public function __construct(Mux $mux, array $options = array())
    {
        $this->mux = $mux;
        $this->options = $options;
    }

    public function dispatchRequest(RouteRequest $request)
    {
        return $this->dispatch($request->getPath(), $request);
    }

    public function dispatch($path) 
    {
        if ($route = $this->mux->dispatch($path)) {
            return $route;
        }
        return false;
    }
}



