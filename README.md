# README #

![EDUC globe](http://hem.bredband.net/markuhli/duc/gfx/educ-globe-140x140.png)

## About ##

Enhanced DUC is a dynamic DNS update client and daemon. Primarily it's
written for use with the DNS services that [NoIP](http://www.noip.com)
provide.  However: the protocol "is in an open format used by other
service providers".  I began working on the project in November
2015. The goal is to create a better update client for UNIX by using
modern C coding practices.

## Cloning ##

To clone the repository use [Git](https://git-scm.com).

    $ git clone https://github.com/uhlin/enhanced-duc.git

## Framework ##

Enhanced DUC depends on the [OpenSSL](https://www.openssl.org)
toolkit. Which means that on, for example, a Debian GNU/Linux system
you need to install a package with name `libssl-dev` before building:

    # aptitude install libssl-dev

## Building and installing ##

    $ cd /path/to/source
    $ make
    $ sudo make install

## Program options ##

    -c           Create a config file and exit. Will prompt.
    -x <path>    Load a config file from a custom location.
    -o           Don't cycle. Update the host(s) only once.
    -B           Run in the background.

Example: `educ_noip -c`

## Good to know ##

* If the program isn't running as a daemon it can be terminated with CTRL+C.
* If port 443 is specified SSL will be enabled.
* Valid ports are 80, 443 and 8245.

## Other notes ##

* Building for GNU/Linux was broken in v1.2 - v1.3.1.
