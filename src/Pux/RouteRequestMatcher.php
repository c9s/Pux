<?php
namespace Pux;

interface RouteRequestMatcher
{
    public function matchConstraints(array $constraints);

    public function matchPath($pattern, & $matches = array());

    public function matchHost($host, & $matches = array());

    public function matchRequestMethod($method);

}


