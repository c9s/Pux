<?php
namespace Pux;
use Pux\Mux;
use ReflectionClass;

class MuxCompiler
{
    public $mux;

    public $idCounter;

    public $callbackValidation = false;

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
        if ( $this->callbackValidation ) {
            $this->validateRouteCallback($routes);
        }
        foreach( $routes as $route ) {
            if ( is_int($route[2]) ) {
                // rewrite submux id
                $submux = $mux->getSubMux( $route[2] );
                $newId = ++$this->idCounter;
                $route[2] = $newId;
                $this->mux->submux[ $newId ] = $submux;
            }
            $this->mux->routes[] = $route;
        }
        return true;
    }

    static public function sort_routes($a, $b) {
        if ( $a[0] && $b[0] ) {
            $a_len = strlen($a[3]['compiled']);
            $b_len = strlen($b[3]['compiled']);
            if ( $a_len == $b_len ) {
                return 0;
            } elseif ( $a_len > $b_len ) {
                return -1;
            } else {
                return 1;
            }
        } elseif ( $a[0] ) {
            return -1;
        } elseif ( $b[0] ) {
            return 1;
        }
        if ( strlen($a[1]) > strlen($b[1]) ) {
            return -1;
        } elseif ( strlen($a[1]) == strlen($b[1]) ) {
            return 0;
        } else {
            return 1;
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

    public function compileReflectionParameters() 
    {
        foreach( $this->mux->routes as $route ) {
            $callback = $route[2];
            list($class, $method) = $callback;

            $refClass = new ReflectionClass($class);
            $refMethod = $refClass->getMethod($method);
            $refParameters = $refMethod->getParameters();

            // HHVM does not support Reflection currently.
            if (! defined('HHVM_VERSION')) {
                // when __reflection is defined, we re-use the reflection info.
                $route[3]['__reflection'] = array();
                foreach( $refParameters as $refParam ) {
                    // ReflectionParameter
                    $param = array(
                        'name' => $refParam->getName(),
                        'position' => $refParam->getPosition(),
                    );
                    if ( $refParam->isDefaultValueAvailable() ) {
                        $param['default'] = $refParam->getDefaultValue();
                        if ( $refParam->isDefaultValueConstant() ) {
                            $param['constant'] = $refParam->getDefaultValueConstantName();
                        }
                    }
                    if ( $optional = $refParam->isOptional() ) {
                        $param['optional'] = $optional;
                    }
                    $route[3]['__reflection']['params'][] = $param;
                }
            }
        }
    }

    /**
     * Compile merged routes to file.
     */
    public function compile($outFile) {
        // compile routes to php file as a cache.
        usort($this->mux->routes, [ 'Pux\\MuxCompiler' , 'sort_routes' ]);

        $code = $this->mux->export();
        return file_put_contents($outFile, "<?php return " . $code . "; /* version */" );
    }
}

