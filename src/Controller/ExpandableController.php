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
    public function expand(array $options = [], $dynamic = false)
    {
        $this->mux = new Mux();
        $mux = $this->mux;
        $routes = ControllerRouteBuilder::build($this);
        foreach ($routes as $route) {
            if ($dynamic) {
                $mux->add($route[0], [$this, $route[1]], array_merge($options, $route[2]));
            } else {
                $mux->add($route[0], [static::class, $route[1]], array_merge($options, $route[2]));
            }
        }

        $mux->sort();
        return $mux;
    }
}
