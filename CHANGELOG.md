# Change Log #
All notable changes to this project will be documented in this file.

## [Unreleased] ##
### Added ###
- Better initialization of variables
- Configuration file `enhanced-duc-config.h` where customizations can
  be made
- An enhanced version of the interpreter

### Changed ###
- All C sources to be 80 columns max
- Certain error messages
- Name of `Strdup_printf` to `strdup_printf`
- Name of `Strings_match()` to `strings_match()`
- Name of `Strtolower` to `strToLower`
- Name of `destroy_config_customValues` to `destroy_config_custom_values`
- Name of `log_die()` to `fatal()`

### Fixed ###
- Only provide `strlcpy()` and `strlcat()` if necessary

## [1.6] - 2016-04-04 ##
- Most recent version before this log file was created. 1.6 is now
  history and a major release is planned.
