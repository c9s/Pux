<?php
namespace Pux\MuxBuilder;
use Pux\Mux;
use Pux\Expandable;

class RESTfulMuxBuilder
{

    protected $mux;

    protected $options = array();

    protected $resources = array();

    public function __construct(Mux & $mux = null, array $options = array())
    {
        $this->mux = $mux ?: new Mux;
        $this->options = array_merge(array( 
            'prefix' => '/',
        ), $options);
    }

    public function addResource($resourceId, Expandable $controller)
    {
        $this->resources[$resourceId] = $controller;
    }


    public function build()
    {
        $prefix = $this->options['prefix'];
        foreach ($this->resources as $resId => $controller) {
            $resourceMux = $controller->expand();
            $path = $prefix . '/' . $resId;
            $this->mux->mount($path, $resourceMux);
        }
        return $this->mux;
    }
}



