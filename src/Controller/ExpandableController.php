<?php
namespace Pux\Controller;

use ReflectionClass;
use ReflectionMethod;
use Universal\Http\HttpRequest;
use Pux\Mux;
use Pux\Expandable;
use Pux\Builder\ControllerRouteBuilder;

class ExpandableController extends Controller implements Expandable
{
    /**
     * @var Pux\Mux
     */
    protected $mux;


    public function getMux()
    {
        return $this->mux;
    }


    /**
     * Expand controller actions to Mux object.
     *
     * @return Mux
     */
    public function expand(array $options = array(), $dynamic = false)
    {
        $this->mux = $mux = new Mux();
        $routes = ControllerRouteBuilder::buildActionRoutes($this);
        foreach ($routes as $route) {
            if ($dynamic) {
                $mux->add($route[0], array($this, $route[1]), array_merge($options, $route[2]));
            } else {
                $mux->add($route[0], array(get_class($this), $route[1]), array_merge($options, $route[2]));
            }
        }
        $mux->sort();
        return $mux;
    }
}
