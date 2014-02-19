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

    public function getActionRoutes() {
        $pairs          = array();
        $actionNames    = array_map(function($method) {
            return preg_replace('/Action$/', '', $method);
        }, $this->getActionMethods());

        foreach ($actionNames as $actionName) {
            if ($actionName === 'index') {
                $path = '';
            } else {
                $path = preg_replace_callback('/[A-Z]/', function($matches) {
                    return '/' . strtolower($matches[0]);
                }, $actionName);
            }
            $pairs[] = array($path, $actionName . 'Action');
        }

        return $pairs;
    }

    public function expand() {
        $mux    = new Mux();
        $paths  = $this->getActionRoutes();
        
        foreach ($paths as $path) {
            $rmth   = new ReflectionMethod($this, $path[1]);
            $doc    = $rmth->getDocComment();
            $opts   = array();

            if ($doc) {
                if (preg_match('/^[\s*]*\@Method\("(get|put|post|delete|head|patch|options)"\)/im', $doc, $mmatch)) {
                    $opts['method'] = $mux->getRequestMethodConstant(array_pop($mmatch));
                }

                if (preg_match('/^[\s*]*\@Route\("([^\s]+)"\)/im', $doc, $umatch)) {
                    $path[0] = ltrim(array_pop($umatch), '/');
                }
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


