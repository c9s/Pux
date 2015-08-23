<?php
namespace Pux\App;
use Pux\Executor;
use Pux\Mux;

class MuxApp
{
    protected $mux;

    public function __construct(Mux $mux = null)
    {
        $this->mux = $mux ?: new Mux;
    }

    public function mount($path, $app)
    {
        $this->mux->any($path, $app);
        return $this;
    }

    public function getMux()
    {
        return $this->mux;
    }

    public function call(array & $environment, array $response)
    {
        if ($route = $this->mux->dispatch($environment['PATH_INFO'])) {
            list($pcre, $pattern, $app, $options) = $route;
            if ($app instanceof Middleware) { 
                return $app($environment, $response);
            } else {
                return Executor::execute($app);
            }
        }
        return $response;
    }

    public function __invoke(array $environment, array $response)
    {
        return $this->call($environment, $response);
    }

    static public function mountWithMap(array $maps)
    {
        $mux = new Mux;
        foreach ($maps as $path => $app) {
            $mux->any($path, $app);
        }
        return new self($mux);
    }


}



