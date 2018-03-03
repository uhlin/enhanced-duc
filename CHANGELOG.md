# Change Log #
All notable changes to this project will be documented in this file.

## [Unreleased] ##
### Added ###
- An enhanced version of the interpreter
- An enhanced version of the man page
- Better initialization of variables
- Configuration file `enhanced-duc-config.h` where customizations can be made
- Many enhancements to the networking routines

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

### Fixed ###
- Only provide `strlcpy()` and `strlcat()` if necessary

## [1.6] - 2016-04-04 ##
- Most recent version before this log file was created. 1.6 is now
  history and a major release is planned.
