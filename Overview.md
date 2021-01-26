# libprom

This project provides shared libraries for instrumenting software and expose metrics using the [Prometheus exposition format](https://prometheus.io/docs/instrumenting/exposition_formats/)

* libprom - Provides the core API. Resources such as counters, gauges, histograms, and
  collector registries can be found here. This library has no dependencies on third-party
  libraries; however, it does rely on pthreads native to POSIX systems.
* libpromhttp - Provides a simple web handler to expose Prometheus metrics for scraping.
  This library has a dependency on libmicrohttpd.

Documentation can be generated using `make docs`.

## Versioning

This project generally follows [semantic versioning](https://semver.org).

## Development

Navigate to the root fo this project directory and execute `make` - it will build libprom, libpromhttp and generate the documentation from the source code. To run all tests, use `make test` and `make smoke`. To generate the source documentation only use `make doc`.

### General Rules for Contribution

* Open An Issue: Before opening a PR or starting any work, open an issue.  In the issue, describe the problem you
want to fix and how you would like to fix it.  The level of detail should match the relative size of the proposed change.
This will allow us to work together to determine the best path forward towards a sound solution.

* Open a Pull Request: After you have gotten confirmation on your proposed change it's time to get to work! Create a
fork and make all of your updates in said fork. For each commit, you must prefix the commit with the associated issue.
For example: `#12 - Fixing typo in documentation`. Before opening a pull request, review the commit log for your fork.
If any of your commit messages are extraneous, squash said commits using `git rebase`. Once you're happy with your
changes and your commit log, open a pull request against the master branch.

* Engage in the Code Review: After submitting your pull request, there may be some requests for changes.  If you have
any questions or concerns, please do not hesitate to make them known.  The code review process is a bidirectional
communication process so please do not be shy. Speak up!

### Coding Rules for Contribution

* Please follow the general coding style already present in the project.
  * Every struct must have a constructor function and destructor function.
  * Every method must pass a pointer to the target struct as the first argument.
  * Every function that is not a constructor or destructor and does not return a value must return an int to signify
    success with 0 and failure with non-zero.
  * Every function name must begin with the library name. For example, all functions within prom must begin with `prom_`
    and all functions within promhttp must begin with `promhttp_`.
  * All variables must be underscore delimited (i.e. snake-case).
  * All macros must be captilalized.
* All new functions should introduce a corresponding suite of unit tests.
* If you add functionality to an existing function, be sure to add a corresponding unit test to verify correctness.

## Misc

* Language level: C11
