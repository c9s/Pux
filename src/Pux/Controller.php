<?php
namespace Pux;
use ReflectionClass;
use ReflectionObject;
use ReflectionMethod;
use Pux\Mux;

class Controller {


    protected function parseMethodAnnotation($refObject, & $args, $parent = 0) {
        if ($pClassRef = $refObject->getParentClass()) {
            $this->parseMethodAnnotation($pClassRef, $args, 1);
        }

        $methods = $refObject->getMethods();
        foreach( $methods as $method ) {
            if ( ! preg_match('/Action$/', $method->getName()) ) {
                continue;
            }

            // $rmth   = new ReflectionMethod($this, $path[1]);
            $doc    = $method->getDocComment();
            $annotations = array();
            $meta = array( 'class' => $refObject->getName() );

            if ($parent) {
                $meta['is_parent'] = true;
            }

            if ($doc) {
                if (preg_match('/^[\s*]*\@Method\("(get|put|post|delete|head|patch|options)"\)/im', $doc, $mmatch)) {
                    $annotations['Method'] = array_pop($mmatch);
                }
                if (preg_match('/^[\s*]*\@Route\("([^\s]*)"\)/im', $doc, $umatch)) {
                    $annotations['Route'] = array_pop($umatch);
                }
            }

            $args[] = array( $method->getName(), $annotations, $meta);
        }
    }



    public function getActionMethods() {
        $refObject = new ReflectionObject($this);
        $args = array();
        $this->parseMethodAnnotation($refObject, $args, 0);
        return $args;
    }

    protected function translatePath($methodName) {
        $methodName = preg_replace('/Action$/', '', $methodName);
        return '/' . preg_replace_callback('/[A-Z]/', function($matches) {
            return '/' . strtolower($matches[0]);
        }, $methodName);
    }


    /**
     * Return [["/path", "testAction", [ "method" => ... ] ],...]
     *
     */
    public function getActionRoutes() {
        $pairs          = array();
        $actions    = $this->getActionMethods();

        foreach ($actions as $actionInfo) {
            list($actionName, $annotations, $meta) = $actionInfo;

            if ( isset($annotations['Route']) ) {
                $path = $annotations['Route'];
            } else {
                if ($actionName === 'indexAction') {
                    $path = '';
                } else {
                    $path = $this->translatePath($actionName); // '/' . preg_replace_callback('/[A-Z]/', function($matches) {
                }
            }

            $pair = array($path, $actionName);

            if (isset($annotations['Method']) ) {
                $pair[] = array( 'method' => Mux::getRequestMethodConstant($annotations['Method']) );
            } else {
                $pair[] = array();
            }
            $pairs[] = $pair;
        }
        return $pairs;
    }

    public function expand() {
        $mux    = new Mux();
        $paths  = $this->getActionRoutes();
        
        foreach ($paths as $path) {
            $mux->add($path[0], array(get_class($this), $path[1]), $path[2]);
        }

        $mux->sort();
        return $mux;
    }

    public function toJson($data) {
        return json_encode($data);
    }

}


