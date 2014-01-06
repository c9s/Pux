<?php
namespace Pux\Command;
use Pux\MuxCompiler;
use Pux\Mux;
use CLIFramework\Command;

class CompileCommand extends Command
{
    public function brief() { return 'compile routes'; }

    public function options($opts) {
        $opts->add('o:', 'output file');
    }

    public function execute()
    {
        $files = func_get_args();
        $outputFile = $this->options->o ?: '_mux.php';

        $compiler = new MuxCompiler();

        foreach( $files as $file ) {
            $compiler->load($file);
        }
        $compiler->compile($outputFile);
    }
}


