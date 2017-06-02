<?php
namespace Pux\MuxBuilder;
use Pux\Mux;
use Pux\Expandable;

class RESTfulMuxBuilder
{

    /**
     * @var Pux\Mux
     */
    protected $mux;

    /**
     * @var array $options
     *
     * Valid options are: 'prefix'
     */
    protected $options = array();


    /**
     * @var array $reousrces
     *
     * [ resource Id => expandable controller object ]
     *
     */
    protected $resources = array();

    public function __construct(Mux & $mux = null, array $options = array())
    {
        $this->mux = $mux ?: new Mux;
        $this->options = array_merge(array( 
            'prefix' => '/',
        ), $options);
    }


    /**
     * Register a RESTful resource into the Mux object.
     *
     * @param string $resourceId the RESTful resource ID
     * @param Expandable $controller a controller object that implements Expandable interface.
     *
     */
    public function addResource($resourceId, Expandable $controller)
    {
        $this->resources[$resourceId] = $controller;

        $prefix = $this->options['prefix'];
        $resourceMux = $controller->expand();
        $path = $prefix . '/' . $resourceId;
        $this->mux->mount($path, $resourceMux);
    }


    /**
     * build method returns a Mux object with registered resources.
     *
     * @return Pux\Mux
     */
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



