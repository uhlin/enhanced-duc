#ifndef OPTION_DESCRIPTION_H
#define OPTION_DESCRIPTION_H
/* Copyright (c) 2021 Markus Uhlin <markus.uhlin@bredband.net>
   All rights reserved.

   Permission to use, copy, modify, and distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
   AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
   PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE. */

static const char USERNAME_DESC[] =
  "Your username.";
static const char PASSWORD_DESC[] =
  "Your password. (Will not echo!)";

static const char HOSTNAME_DESC[] =
  "The hostname to be updated. (Multiple hosts are separated with a vertical\n"
  "bar.)";
static const char IP_ADDR_DESC[] =
  "Associate the hostname(s) with this IP address. If the special value\n"
  "'WAN_address' is specified, the associated IP address will be the WAN\n"
  "address of the computer that the DUC is running on.";

static const char SP_HOSTNAME_DESC[] =
  "Service provider hostname. (The update request is sent to this hostname\n"
  "or IP.)";
static const char PORT_DESC[] =
  "Target port? (443 = enable TLS/SSL.)";

static const char UPDATE_INTERVAL_SECONDS_DESC[] =
  "Update interval in seconds. If a value less than 600 is entered, the\n"
  "program will fallback to 1800 in order to avoid flooding the server with\n"
  "requests.";

static const char PRIMARY_IP_LOOKUP_SRV_DESC[] =
  "Server used to determine your external IP.";
static const char BACKUP_IP_LOOKUP_SRV_DESC[] =
  "Backup server for IP lookups.";

static const char FORCE_UPDATE_DESC[] =
  "Even if your external IP address hasn't changed between update intervals,\n"
  "OR if the program cannot determine your external IP -- in either way:\n"
  "force update. This setting should be set to YES if 'ip_addr' not equals\n"
  "to 'WAN_address'.";

#endif
