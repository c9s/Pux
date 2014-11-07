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
        $key = $this->buildKey($path);

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

    protected function buildKey($path) {
        $method = isset($_SERVER['REQUEST_METHOD']) ? $_SERVER['REQUEST_METHOD'] : 'GET';
        $host = isset($_SERVER["HTTP_HOST"]) ? $_SERVER["HTTP_HOST"] : '';
        $https = isset($_SERVER["HTTPS"]) ? $_SERVER["HTTPS"] : '';

        $key = $this->getNamespace() . ':' . (string)$https . (string)$host . ':' . (string)$method . ':' . $path;

        return $key;
    }
}



