<?php

namespace Pux;
use ReflectionClass;
use ReflectionObject;
use ReflectionMethod;
use Pux\Mux;


class Controller {
    public function getActionMethods() {
        $refObject = new ReflectionObject($this);
        return array_map(function($m) { return $m->getName(); }, array_filter($refObject->getMethods(), function($m) { 
            return preg_match('/Action$/', $m->getName());
        }));
    }

    public function getActionPaths() {
        $actionNames = array_map(function($method) {
            return preg_replace('/Action$/', '', $method);
        }, $this->getActionMethods());

        $pairs = array();
        foreach($actionNames as $actionName) {
            $httpMethod = null;
            $actionName = preg_replace_callback('/(Get|Put|Post|Delete)$/', function($matches) use(&$httpMethod) { 
                $httpMethod = $matches[0];
                return;
            }, $actionName);

            if($actionName == "index") {
                $path = '';
            } else {
                $path = '/' . preg_replace_callback('/[A-Z]/', function($matches) { 
                    return '/' . strtolower($matches[0]);
                }, $actionName);
            }

            $pairs[] = array($path, $actionName . $httpMethod . 'Action', strtoupper($httpMethod));
        }

        return $pairs;
    }

    public function expand() {
        $mux = new \Pux\Mux;
        $paths = $this->getActionPaths();

        foreach($paths as $path) {
            $opts = array();

            if(isset($path[2])) {
                $httpMethod = $mux->getRequestMethodConstant(strtoupper($path[2]));
                $httpMethod && ($opts['method'] = $httpMethod);
            }

            $mux->add($path[0], array(get_class($this), $path[1]), $opts);
        }

        $mux->sort();
        return $mux;
    }

    public function toJson($data) {
        return json_encode($data);
    }
}
