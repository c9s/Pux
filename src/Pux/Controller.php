<?php
namespace Pux;
use ReflectionClass;
use ReflectionObject;
use ReflectionMethod;
use Pux\Mux;

class Controller {

    protected function parseMethodAnnotation($method) {

        $annotations = array();
        $doc = $method->getDocComment();
        if ($doc) {
            if (preg_match('/^[\s*]*\@Method\("(get|put|post|delete|head|patch|options)"\)/im', $doc, $regs)) {
                $annotations['Method'] = $regs[1];
            }
            if (preg_match('/^[\s*]*\@Route\("([^\s]*)"\)/im', $doc, $regs)) {
                $annotations['Route'] = $regs[1];
            }
        }
        return $annotations;
    }

    protected function parseMethods($refObject, & $args, $parent = 0) {
        if ($pClassRef = $refObject->getParentClass()) {
            $this->parseMethods($pClassRef, $args, 1);
        }

        $methods = $refObject->getMethods();
        foreach( $methods as $method ) {
            if ( ! preg_match('/Action$/', $method->getName()) ) {
                return;
            }

            $meta = array( 'class' => $refObject->getName() );
            $anns = $this->parseMethodAnnotation($method);
            if (empty($anns)) {
                // get parent method annotations
                if (isset($args[ $method->getName() ]) ) {
                    $anns = $args[$method->getName()][0];
                }
            }
            // override
            $args[ $method->getName() ] = array( $anns, $meta );
        }
    }



    public function getActionMethods() {
        $refObject = new ReflectionObject($this);
        $args = array();
        $this->parseMethods($refObject, $args, 0);
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

        foreach ($actions as $actionName => $actionInfo) {
            list($annotations, $meta) = $actionInfo;

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


