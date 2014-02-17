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
        $tuples         = array();
        $actionNames    = $this->getActionMethods();

        foreach ($actionNames as $actionName) {
            $rmth   = new ReflectionMethod($this, $actionName);
            $doc    = $rmth->getDocComment();

            if(!$doc) {
                continue;
            }

            $camelUrlAction = substr($actionName, 0, -6);
            $path = $method = null;

            if(preg_match('/^[\s*]*\@method (get|put|post|delete|head|patch|options)\s*$/im', $doc, $mmatch)) {
                $method = array_pop($mmatch);
            }

            if(preg_match('/^[\s*]*\@uri ([^\s]+)\s*$/im', $doc, $umatch)) {
                $path = ltrim(array_pop($umatch), '/');
            } else if($camelUrlAction !== 'index') {
                $path = preg_replace_callback('/[A-Z]/', function($regs) { 
                    return '/' . strtolower($regs[0]);
                }, $camelUrlAction);
            }

            $tuples[] = array(ltrim($path, '/'), $actionName, $method);
        }

        return $tuples;
    }

    public function expand() {
        $mux    = new Mux();
        $paths  = $this->getActionPaths();

        foreach ($paths as $path) {
            $opts = array();
            isset($path[2]) && ($opts['method'] = $mux->getRequestMethodConstant($path[2]));
            $mux->add($path[0], array(get_class($this), $path[1]), $opts);
        }

        $mux->sort();
        return $mux;
    }

    public function toJson($data) {
        return json_encode($data);
    }

}


