<?php
namespace Pux;

interface RouteRequestMatcher
{
    public function matchConstraints(array $constraints);

    public function matchPath($pattern, & $matches = array());

    public function matchHost($host, & $matches = array());

    public function matchRequestMethod($method);

    public function matchPathSuffix($suffix);

    public function containsPath($path);

    public function equalsHost($host);

    public function equalsPath($path);

    public function equalsPort($port);

    public function matchQueryString($pattern, & $matches = array());

}


