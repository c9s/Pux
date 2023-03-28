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
    protected $options = [];


    /**
     * @var array $reousrces
     *
     * [ resource Id => expandable controller object ]
     *
     */
    protected $resources = [];

    public function __construct(Mux & $mux = null, array $options = [])
    {
        $this->mux = $mux ?: new Mux;
        $this->options = array_merge(['prefix' => '/'], $options);
    }


    /**
     * Register a RESTful resource into the Mux object.
     *
     * @param string $resourceId the RESTful resource ID
     * @param Expandable $expandable a controller object that implements Expandable interface.
     */
    public function addResource($resourceId, Expandable $expandable)
    {
        $this->resources[$resourceId] = $expandable;

        $prefix = $this->options['prefix'];
        $resourceMux = $expandable->expand();
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



