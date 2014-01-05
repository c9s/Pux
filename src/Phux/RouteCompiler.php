<?php
namespace Phux;
use Phux\Mux;

class RouteCompiler
{
    public $mux;

    public $idCounter;

    public function __construct() {
        $this->idCounter = 0;
        $this->mux = new Mux;
    }

    public function get() {
        return $this->mux;
    }

    public function set($mux) {
        $this->mux = $mux;
    }

    public function merge($mux) {
        $routes = $mux->getRoutes();
        $this->validateRouteCallback($routes);
        foreach( $routes as $route ) {
            if ( is_int($route[2]) ) {
                // rewrite submux id
                $submux = $mux->getSubMux( $route[2] );
                $newId = ++$this->idCounter;
                $route[2] = $newId;
                $this->mux->subMux[ $newId ] = $submux;
            }
            $this->mux->routes[] = $route;
        }
        return true;
    }

    static public function sort_routes($a, $b) {
        if ( $a[0] && $b[0] ) {
            return strlen($a[3]['compiled']) > strlen($b[3]['compiled']);
        } elseif ( $a[0] ) {
            return 1;
        } elseif ( $b[0] ) {
            return -1;
        }
        if ( strlen($a[1]) > strlen($b[1]) ) {
            return 1;
        } elseif ( strlen($a[1]) == strlen($b[1]) ) {
            return 0;
        } else {
            return -1;
        }
    }

    /**
     * @param string $routeFile routeFile, return the Mux
     */
    public function load($muxFile) {
        $mux = require $muxFile;
        return $this->merge($mux);
    }

    /**
     * validate controller classes and controller methods before compiling to 
     * route cache.
     */
    public function validateRouteCallback($routes) 
    {
        foreach( $routes as $route ) {
            $callback = $route[2];
            if ( is_array($callback) ) {
                $class = $callback[0];
                $method = $callback[1];
                if ( ! class_exists($class, true) ) {
                    throw new Exception("Controller {$class} does not exist.");
                }
                // rebless a controller (extract this to common method)
                $controller = new $class;
                if ( ! method_exists($controller, $method) ) {
                    throw new Exception("Method $method not found in controller $class.");
                }
            }
        }
    }

    /**
     * Compile merged routes to file.
     */
    public function compile($outFile) {
        // compile routes to php file as a cache.
        usort($this->mux->routes, [ 'Phux\\RouteCompiler' , 'sort_routes' ]);

        $code = $this->mux->export();
        return file_put_contents($outFile, "<?php return " . $code . "; /* version */" );
    }
}

