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

    public function add() 
    {
        // $this->mux->add( func_get_args() );
    }

    public function cacheInvalid() 
    {
        return true;
    }


    public function save() {

    }

    public function dispatch($path) {

    }
}

