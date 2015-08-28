<?php
namespace Pux;

/**
 * Environment array factory method
 *
 *    Environment::createFromGlobals($GLOBALS);
 */
class Environment
{

    /**
     * Create from globals array
     *
     * @param array $array
     * @return array
     */
    static public function createFromArray(array $array)
    {
        $env = $array['_SERVER'];
        $env['_REQUEST'] = $array['_REQUEST'];
        $env['_POST']    = $array['_POST'];
        $env['_GET']     = $array['_GET'];
        $env['_COOKIE']  = $array['_COOKIE'];
        $env['_SESSION'] = $array['_SESSION'];
        return $env;
    }


    static public function createFromGlobals()
    {
        $env = $GLOBALS['_SERVER'];
        $env['_REQUEST'] = $GLOBALS['_REQUEST'];
        $env['_POST']    = $GLOBALS['_POST'];
        $env['_GET']     = $GLOBALS['_GET'];
        $env['_COOKIE']  = $GLOBALS['_COOKIE'];
        $env['_SESSION'] = $GLOBALS['_SESSION'];
        return $env;
    }
}





