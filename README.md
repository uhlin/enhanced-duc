# README #

![EDUC globe](educ-globe-140x140.png)

[![Coverity Scan Build Status](https://scan.coverity.com/projects/18259/badge.svg)](https://scan.coverity.com/projects/enhanced-duc)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/uhlin/enhanced-duc.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/uhlin/enhanced-duc/context:cpp)

## About ##

Enhanced DUC is a dynamic DNS update client and daemon. Primarily it's
written for use with the DNS services that [NoIP](http://www.noip.com)
provide. However: the protocol "is in an open format used by other
service providers". I began working on the project in November
2015\. The goal is to create a better update client for UNIX by using
modern C coding practices.

## Cloning ##

To clone the repository use [Git](https://git-scm.com).

    $ git clone https://github.com/uhlin/enhanced-duc.git

## Framework ##

Enhanced DUC depends on the [OpenSSL](https://www.openssl.org)
toolkit. Which means that on, for example, a Debian GNU/Linux system
you need to install a package with name `libssl-dev` before building:

    # apt install libssl-dev

## Building and installing ##

    $ cd /path/to/source

If your system comes with `strlcpy()` and `strlcat()` please edit
`enhanced-duc-config.h`.

    $ make
    $ sudo make install

## Program options ##

    -h           Print help
    -c           Create a config file by asking the user for input.
                 The user will be given the opportunity to choose a
                 location for the config file, i.e. where to create it.
    -x <path>    Start the DUC with the config file specified by path
    -D           Turn on debug mode
    -o           Don't cycle, i.e. don't periodically check for IP
                 changes. Only update the hostname(s) once.
    -B           Run in the background and act as a daemon

## Good to know ##

* If the program isn't running as a daemon it can be terminated with CTRL+C.
* If port 443 is specified TLS/SSL will be enabled.
* Valid ports are 80, 443 and 8245.

## Other notes ##

* Building for GNU/Linux was broken in v1.2 - v1.3.1.
* Please read the system log, possibly `/var/log/daemon.log` or
  `/var/log/daemon` (depending on your UNIX variant).
