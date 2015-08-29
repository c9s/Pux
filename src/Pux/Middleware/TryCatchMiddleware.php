<?php
namespace Pux\Middleware;

class TryCatchMiddleware extends Middleware
{
    protected $options = array();

    public function __construct(callable $app, array $options = array())
    {
        parent::__construct($app);
        $this->options = $options;
    }


    public function call(array & $environment, array $response)
    {
        try {
            return parent::call($environment, $response);
        } catch (Exception $e) {
            if (isset($this->options['throw'])) {
                throw $e;
            }
        }
        return $response;
    }
}

