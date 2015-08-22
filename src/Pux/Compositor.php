<?php
namespace Pux;
use ReflectionClass;

class Compositor
{
    protected $stacks = array();

    protected $app;

    public function enable($appClass)
    {
        $args = func_get_args();
        array_shift($args);
        $this->stacks[] = [$appClass, $args];
        return $this;
    }

    public function app($app)
    {
        $this->app = $app;
        return $this;
    }

    public function wrap()
    {
        $app = $this->app;

        for ($i = count($this->stacks) - 1; $i > 0; $i--) {
            list($appClass, $args) = $this->stacks[$i];
            $refClass = new ReflectionClass($appClass);
            array_unshift($args, $app);
            $app = $refClass->newInstanceArgs($args);
        }
        return $app;
    }
}



