<?php

namespace Pux;

class HttpMethodController extends Controller {
    public function expand() {
        $mux = parent::expand();

        $mux->staticRoutes = array();
        foreach($mux->routes as &$route) {
            $rmth   = new \ReflectionMethod($this, $route[2][1]);
            $doc    = $rmth->getDocComment();

            if(!$doc) {
                continue;
            }

            preg_match('/^[\s*]*\@method (get|put|post|delete|head|patch|options)\s*$/im', $doc, $mmatch);
            $mmatch && ($route[3]['method'] = $mux->getRequestMethodConstant(strtoupper(array_pop($mmatch))));

            preg_match('/^[\s*]*\@uri ([^\s]+)\s*$/im', $doc, $umatch);
            $umatch && ($route[1] = ltrim(array_pop($umatch), '/'));
        }

        return $mux;
    }
}
