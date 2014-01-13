<?php
namespace Pux;
use ReflectionClass;
use ReflectionObject;
use ReflectionMethod;
use Pux\Mux;

class Controller {

    public function getActionMethods() {
        $refObject = new ReflectionObject($this);
        return array_map(function($m){ return $m->getName(); }, array_filter($refObject->getMethods(), function($m) { 
            return preg_match('#Action$#', $m->getName() );
        }));
    }

    public function getActionPaths() {
        $actionNames = array_map(function($method) {
            return preg_replace('#Action$#','', $method);
        }, $this->getActionMethods() );

        $pairs = array();
        foreach( $actionNames as $actionName ) {
            if ( $actionName == "index" ) {
                $path = '';
            } else {
                $path = '/' . preg_replace_callback('/[A-Z]/', function($regs) { 
                    return '/' . strtolower($regs[0]);
                }, $actionName);
            }
            $pairs[] = array($path, $actionName);
        }
        return $pairs;
    }

    public function expand() {
        $mux = new Mux;
        $paths = $this->getActionPaths();
        foreach( $paths as $path ) {
            $mux->add($path[0], array(get_class($this), $path[1]) );
        }
        $mux->sort();
        return $mux;
    }

}


