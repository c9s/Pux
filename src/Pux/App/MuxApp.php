<?php
namespace Pux\App;
use Pux\RouteExecutor;
use Pux\Mux;
use Pux\Middleware;
use Pux\App;
use Pux\Compositor;

class MuxApp implements App
{
    protected $mux;

    public function __construct(Mux $mux = null)
    {
        $this->mux = $mux ?: new Mux;
    }

    /**
     * Mount app on a specific path
     *
     * @param string $path
     * @param App $app
     * @return MuxApp
     */
    public function mount($path, App $app)
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

            if ($app instanceof App || $app instanceof Middleware) { 

                return $app($environment, $response);

            } else {

                return RouteExecutor::execute($route);

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
            if ($app instanceof Compositor) {
                $app = $app->wrap();
            }
            $mux->any($path, $app);
        }
        return new self($mux);
    }


}



