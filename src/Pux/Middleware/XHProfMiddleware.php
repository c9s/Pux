<?php
namespace Pux\Middleware;
use XHProfRuns_Default;
use LogicException;

class XHProfMiddleware extends Middleware
{
    protected $options = array();

    protected $runId;

    public function __construct(callable $app, array $options = array())
    {
        parent::__construct($app);
        $this->options = array_merge([
            'flags' => XHPROF_FLAGS_CPU + XHPROF_FLAGS_MEMORY,
            'prefix' => 'pux_',
            'output_dir' => ini_get('xhprof.output_dir') ?: '/tmp',
            'root' => null,
        ],$options);

        if (!$this->options['root']) {
            throw new LogicException("xhprof root is not defined.");
        }
        include_once $this->options['root'] . "/xhprof_lib/utils/xhprof_lib.php";
        include_once $this->options['root'] . "/xhprof_lib/utils/xhprof_runs.php";
    }

    public function getLastRunId()
    {
        return $this->runId;
    }

    public function call(array & $environment, array $response)
    {
        $namespace = $this->options['prefix'];
        if (isset($environment['PATH_INFO'])) {
            $namespace .= $environment['PATH_INFO'];
        } else if (isset($environment['REQUEST_URI'])) {
            $namespace .= $environment['REQUEST_URI'];
        }
        $namespace = preg_replace('#[^\w]+#','_', $namespace);

        xhprof_enable($this->options['flags']);
        $response = parent::call($environment, $response);
        $profile = xhprof_disable();


        $runs = new XHProfRuns_Default($this->options['output_dir']);
        $this->runId = $runs->save_run($profile, $namespace);
        return $response;
    }
}
