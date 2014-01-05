<?php
namespace Phux;
use CLIFramework\Application;

class Console extends Application
{
    const NAME = 'phpux';
    const VERSION = "1.1.0";

    public function init()
    {
        parent::init();
        $this->registerCommand('compile');
    }

    public function brief()
    {
        return 'Phux - High Performance PHP Router.';
    }
}
