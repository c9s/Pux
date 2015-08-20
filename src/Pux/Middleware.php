<?php
namespace Pux;

class Middleware
{
    /**
     * @var Middleware
     */
    public $next;

    public function __construct(callable $next)
    {
        $this->next = $next;
    }

    public function call($env, $res)
    {
        try {
            if ($n = $this->next) {
                $res = $n($env, $res);
            }
            // $res = $next($env, $res);
        } catch (Exception $e) {

        }
        return $res;
    }

    public function __invoke($env, $res)
    {
        $n = $this->next;
        return $n($env, $res);
    }
    
}



