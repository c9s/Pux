<?php
namespace Pux;

interface RouteRequestMatcher
{
    public function matchConstraints(array $constraints);

    public function pathEqual($path);

    public function pathMatch($pattern, array & $matches = array());

    public function pathEndWith($suffix);


    /*
    public function matchPath($pattern, & $matches = array());
    */

    public function hostMatch($host, array & $matches = array());

    public function requestMethodEqual($method);


    public function pathContain($path);

    public function hostEqual($host);

    public function portEqual($port);

    public function queryStringMatch($pattern, array & $matches = array());

}


