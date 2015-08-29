<?php
namespace Pux\Dispatcher;
use Pux\Mux;

class Dispatcher implements Dispatchable
{
    protected $options = array();

    protected $mux;

    protected $namespace = 'app';

    public $expiry = 0;

    public function __construct(Mux $mux, array $options = array())
    {
        $this->mux = $mux;
        $this->options = $options;
        if (isset($this->options['namespace'])) {
            $this->namespace = $this->options['namespace'];
        }
        if (isset($this->options['expiry'])) {
            $this->expiry = $this->options['expiry'];
        }
    }

    public function dispatch($path) 
    {
        if ($route = $this->mux->dispatch($path) ) {
            return $route;
        }
        return false;
    }
}



