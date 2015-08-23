<?php
namespace Pux\Middleware;
use Pux\Middleware;

class CORSService
{
    private $options;
    public function __construct(array $options = array())
    {
        $this->options = $this->normalizeOptions($options);
    }
    private function normalizeOptions(array $options = array())
    {
        $options += array(
            'allowedOrigins' => array(),
            'supportsCredentials' => false,
            'allowedHeaders' => array(),
            'exposedHeaders' => array(),
            'allowedMethods' => array(),
            'maxAge' => 0,
        );
        // normalize array('*') to true
        if (in_array('*', $options['allowedOrigins'])) {
            $options['allowedOrigins'] = true;
        }
        if (in_array('*', $options['allowedHeaders'])) {
            $options['allowedHeaders'] = true;
        } else {
            $options['allowedHeaders'] = array_map('strtolower', $options['allowedHeaders']);
        }
        if (in_array('*', $options['allowedMethods'])) {
            $options['allowedMethods'] = true;
        } else {
            $options['allowedMethods'] = array_map('strtoupper', $options['allowedMethods']);
        }
        return $options;
    }
    public function isActualRequestAllowed(array $environment)
    {
        return $this->checkOrigin($environment);
    }

    public function isCorsRequest(array $environment)
    {
        return isset($environment['HTTP_ORIGIN']);
    }

    public function isPreflightRequest(array $environment)
    {
        // if (isset($_SERVER['HTTP_ACCESS_CONTROL_REQUEST_METHOD']))
        return $this->isCorsRequest($environment)
            && $environment['REQUEST_METHOD'] == 'OPTIONS'
            && isset($environment['HTTP_ACCESS_CONTROL_REQUEST_METHOD']);
    }

    public function addActualRequestHeaders(array $environment, array $response)
    {
        if (! $this->checkOrigin($environment)) {
            return $response;
        }
        $response[1][] = [ 'Access-Control-Allow-Origin' => $request->headers->get('Origin') ];
        /*
        if (! $response->headers->has('Vary')) {
            $response->headers->set('Vary', 'Origin');
        } else {
            $response->headers->set('Vary', $response->headers->get('Vary') . ', Origin');
        }
         */
        if ($this->options['supportsCredentials']) {
            $response[1][] = ['Access-Control-Allow-Credentials' => 'true'];
        }
        if ($this->options['exposedHeaders']) {
            $response[1][] = ['Access-Control-Expose-Headers' => implode(', ', $this->options['exposedHeaders'])];
        }
        return $response;
    }
    public function handlePreflightRequest(array $environment)
    {
        if (true !== $check = $this->checkPreflightRequestConditions($environment)) {
            return $check;
        }
        return $this->buildPreflightCheckResponse($environment);
    }

    private function buildPreflightCheckResponse(array $environment)
    {
        $response = [200,[],[]];
        if ($this->options['supportsCredentials']) {
            $response[1][] = array('Access-Control-Allow-Credentials' => 'true');
        }
        $response[1][] = array('Access-Control-Allow-Origin' => $request->headers->get('Origin'));

        if ($this->options['maxAge']) {
            $response[1][] = array('Access-Control-Max-Age' => $this->options['maxAge']);
        }

        $allowMethods = $this->options['allowedMethods'] === true
            ? strtoupper($environment['HTTP_ACCESS_CONTROL_REQUEST_METHOD'])
            : implode(', ', $this->options['allowedMethods']);
        $response->headers->set('Access-Control-Allow-Methods', $allowMethods);
        $allowHeaders = $this->options['allowedHeaders'] === true
            ? strtoupper($environment['HTTP_ACCESS_CONTROL_REQUEST_HEADERS'])
            : implode(', ', $this->options['allowedHeaders']);

        $response[1][] = array('Access-Control-Allow-Headers' => $allowHeaders);
        return $response;
    }


    private function checkPreflightRequestConditions(array $environment)
    {
        if ( ! $this->checkOrigin($environment)) {
            return $this->createBadRequestResponse(403, 'Origin not allowed');
        }
        if ( ! $this->checkMethod($environment)) {
            return $this->createBadRequestResponse(405, 'Method not allowed');
        }
        $requestHeaders = array();
        // if allowedHeaders has been set to true ('*' allow all flag) just skip this check
        if ($this_>options['allowedHeaders'] !== true && isset($environment['HTTP_ACCESS_CONTROL_REQUEST_HEADERS'])) {
            $headers        = strtolower($environment['HTTP_ACCESS_CONTROL_REQUEST_HEADERS']);
            $requestHeaders = explode(',', $headers);
            foreach ($requestHeaders as $header) {
                if (! in_array(trim($header), $this->options['allowedHeaders'])) {
                    return $this->createBadRequestResponse(403, 'Header not allowed');
                }
            }
        }
        return true;
    }

    private function createBadRequestResponse($code, $reason = '')
    {
        return [ $code, [], $reason];
    }

    private function checkOrigin(array $environment)
    {
        if ($this->options['allowedOrigins'] === true) {
            // allow all '*' flag
            return true;
        }
        $origin = $environment['HTTP_ORIGIN'];
        return in_array($origin, $this->options['allowedOrigins']);
    }

    private function checkMethod(array $environment) {
        if ($this->options['allowedMethods'] === true) {
            // allow all '*' flag
            return true;
        }
        $requestMethod = strtoupper($environment['HTTP_ACCESS_CONTROL_REQUEST_METHOD']);
        return in_array($requestMethod, $this->options['allowedMethods']);
    }
}


class CORSMiddleware extends Middleware
{
    private $cors;

    private $defaultOptions = array(
        'allowedHeaders'      => array(),
        'allowedMethods'      => array(),
        'allowedOrigins'      => array(),
        'exposedHeaders'      => false,
        'maxAge'              => false,
        'supportsCredentials' => false,
    );

    public function __construct($app, array $options = array())
    {
        parent::__construct($app);
        $this->cors = new CORSService(array_merge($this->defaultOptions, $options));
    }

    public function call(array & $environment, array $response)
    {
        if (! $this->cors->isCorsRequest($environment)) {
            return parent::call($environment, $response);
        }

        if ($this->cors->isPreflightRequest($environment)) {
            return $this->cors->handlePreflightRequest($environment);
        }
        if ( ! $this->cors->isActualRequestAllowed($environment)) {
            return [403, [], 'Not allowed'];
        }
        $response = parent::call($environment, $response);
        return $this->cors->addActualRequestHeaders($environment, $response);
    }
}
