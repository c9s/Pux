<?php
namespace Phux;
use Phux\Mux;

class Router
{
    public $cacheFile = 'mux.php';

    public $mux;

    public function __construct($args = array() ) {
        if ( isset($args['cache_file']) ) {
            $this->cacheFile = $args['cache_file'];
        }
        $this->mux = new Mux;
    }

    // dispatch methods to main Mux
    public function __call($method,$args) {
        return call_user_func_array(array($this->mux, $method), $args);
    }

    public function getMux() {
        return $this->mux;
    }

    public function load() 
    {
        if ( file_exists($this->cacheFile) ) {
            $this->mux = require $this->cacheFile;
            return true;
        }
        return false;
    }


    public function save() {
        return $this->compile($this->cacheFile);
    }

    public function dispatch($path) {
        return $this->mux->dispatch($path);
    }
}

