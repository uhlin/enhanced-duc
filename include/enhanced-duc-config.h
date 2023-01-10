#ifndef ENHANCED_DUC_CONFIG_H
#define ENHANCED_DUC_CONFIG_H

/*
 * Which user shall we operate as after dropping superuser (root)
 * privileges?
 */
#define DUC_USER "nobody"

/*
 * Working directory for DUC_USER
 */
#define DUC_DIR "/tmp"

/*
 * Set the update script that's used in the raw HTTP header GET
 * request
 */
#define UPDATE_SCRIPT "/nic/update"

#include "funcs-yesno.h"

/*
 * Set to 0 if the OpenSSL implementation provided by your system
 * doesn't have X509_check_host()
 */
#define HAVE_X509_CHECK_HOST 1

#endif
