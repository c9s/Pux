<?php
namespace Pux\Dispatcher;
use Pux\Mux;

class APCDispatcher
{
    public $options = array();

    public $mux;

    public $namespace = 'app';

    public $expiry = 0;

    public function __construct(Mux $mux, $options = array())
    {
        $this->mux = $mux;
        $this->options = $options;
        if ( isset($this->options['namespace']) ) {
            $this->namespace = $this->options['namespace'];
        }
        if ( isset($this->options['expiry']) ) {
            $this->expiry = $this->options['expiry'];
        }
    }

    public function getNamespace()
    {
        return $this->namespace;
    }

    public function getExpiry()
    {
        return $this->expiry;
    }

    public function dispatch($path) 
    {
        $key = $this->getNamespace() . ':' . $path;
        if ( ($route = apc_fetch($key)) !== false ) {
            apc_inc('hits:'.$key); // record the hits
            return $route;
        }
        if ( $route = $this->mux->dispatch($path) ) {
            apc_store($key, $route, $this->expiry);
            return $route;
        }
        apc_store($key, false, $this->expiry);
        return false;
    }
}



