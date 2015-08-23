<?php
namespace Pux\App;
use Pux\Executor;
use Pux\Mux;
use Pux\Middleware;

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
            $path = $route[1];
            $app = $route[2];

            // Save the original PATH_INFO in ORIG_PATH_INFO
            // Note that some SAPI implementation will save 
            // use the ORIG_PATH_INFO (not noticed yet)
            if (!isset($environment['ORIG_PATH_INFO'])) {
                $environment['ORIG_PATH_INFO'] = $environment['PATH_INFO'];
            }
            $environment['PATH_INFO'] = substr($environment['PATH_INFO'], strlen($path));

            if ($app instanceof Middleware) { 


                return $app($environment, $response);
            } else {
                return Executor::execute($route);
            }
        }
        return $response;
    }

    public function __invoke(array $environment, array $response)
    {
        return $this->call($environment, $response);
    }

    static public function mountWithUrlMap(array $map)
    {
        $mux = new Mux;
        foreach ($map as $path => $app) {
            $mux->any($path, $app);
        }
        return new self($mux);
    }


}



