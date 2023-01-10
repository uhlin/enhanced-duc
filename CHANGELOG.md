# Change Log #
All notable changes to this project will be documented in this file.

## [Unreleased] ##
- Added paths to unveil whitelist
- Added usage of `__func__`
- Changed the directory structure
- Updated the interpreter

## [2.2] - 2021-04-09 ##
- Added and deleted tests
- **Added** colors.h and made use of it
- **Added** compiler flags
- **Added** daemonize.h
- **Added** ducdef.h and made use of its definitions through the whole
  project.
- **Added** make target "check"
- **Added** optdesc.h
- **Added** template.conf
- **Added** usage of unveil()
- **Deleted** def.h
- **Deleted** example.conf
- **Fixed** non ANSI function declarations
- Improved the makefiles
- Indented and formatted **all files** according to OpenBSD's Kernel
  Normal Form (KNF) with a few exceptions.
- **Made improvements to several files and their functions**
- Moved defines
- Moved functions
- Renamed functions and patterns
- **Updated** the man page
- **Updated** the readme

## [2.1] - 2019-06-16 ##
### Added ###
- The possibility to set
  - DUC user
  - DUC dir
  - Update script

### Changed ###
- **Do not stop updating on the first unsuccessful update attempt**!
- **Do not terminate the DUC on**:
  - Unknown server response
- **The config file interpreter**

### Fixed ###
- **Update hostname bug due to an insufficient approach of storing the
  server response**!
- Misspelled words
- Non ANSI function declarations

## [2.0.1] - 2018-04-14 ##
### Added ###
- Unit tests for
  - `net_ssl_check_hostname()`

### Fixed ###
- TLS/SSL hostname verification

## [2.0] - 2018-03-12 ##
### Added ###
- An enhanced version of the interpreter
- An enhanced version of the man page
- Better initialization of variables
- Configuration file `enhanced-duc-config.h` where customizations can be made
- Many enhancements to the networking routines
- New makefiles
- Unit tests for
  - `is_numeric()`
  - `size_product()`
  - `strToLower()`
  - `strdup_printf()`
  - `trim()`
  - `xstrdup()`

### Changed ###
- All C sources to be 80 columns max
- Certain error messages
- Global configuration file from `/etc/educ_noip.conf` to `/etc/enhanced-duc.conf`
- In settings.c: constraints of `is_hostname_ok()`
- Multiple log messages
- Name of `Strdup_printf` to `strdup_printf`
- Name of `Strings_match()` to `strings_match()`
- Name of `Strtolower` to `strToLower`
- Name of `destroy_config_customValues` to `destroy_config_custom_values`
- Name of `log_die()` to `fatal()`
- PID file from `/var/run/educ_noip.pid` to `/var/run/enhanced-duc.pid`
- While creating the config file: don't exit on invalid input

### Fixed ###
- Only provide `strlcpy()` and `strlcat()` if necessary
- TLS/SSL hostname verification

## [1.6] - 2016-04-04 ##
- Most recent version before this log file was created. 1.6 is now
  history and a major release is planned.
