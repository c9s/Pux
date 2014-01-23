CHANGELOG
==================

Version 1.4.0

- Add `pux_persistent_dispatch` function for persistent dispatching.
- Dispatch performance optimization, especially `__set_state` method call, 1.5x faster than before.

Version 1.3.3

- Fix dispatch rtrim bug for pure PHP version.

Version 1.3.2

- phar file re-compiled.

Version 1.3.1

- dispatch performance improvement.
- `__set_state` construction time improvement.
- fix various bugs for zts (zend thread safe mode enabled)
