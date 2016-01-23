# README #

## About ##

Enhanced DUC is an update client primarily written for use with the
DNS solutions that [NoIP](http://www.noip.com) provide. I began
working on the project in November 2015. The goal is to create a
better update client for UNIX by using modern C coding practices.

## Cloning ##

To clone the repository use [Mercurial](http://mercurial.selenic.com/).

    $ hg clone https://bitbucket.org/ma_uhl/enhanced-duc

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
    -B           Run in the background.

Example: `educ_noip -c`

## Good to know ##

* If the program isn't running as a daemon it can be terminated with CTRL+C.
* If port 443 is specified SSL will be enabled.
