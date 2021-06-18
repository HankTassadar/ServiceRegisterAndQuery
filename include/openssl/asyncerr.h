/*
 * Generated by util/mkerr.pl DO NOT EDIT
 * Copyright 1995-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef OPENSSL_ASYNCERR_H
# define OPENSSL_ASYNCERR_H
# pragma once

# include <openssl/opensslconf.h>
# include <openssl/symhacks.h>
# include <openssl/cryptoerr_legacy.h>



/*
 * ASYNC function codes.
 */
# ifndef OPENSSL_NO_DEPRECATED_3_0
#  define ASYNC_F_ASYNC_CTX_NEW                            0
#  define ASYNC_F_ASYNC_INIT_THREAD                        0
#  define ASYNC_F_ASYNC_JOB_NEW                            0
#  define ASYNC_F_ASYNC_PAUSE_JOB                          0
#  define ASYNC_F_ASYNC_START_FUNC                         0
#  define ASYNC_F_ASYNC_START_JOB                          0
#  define ASYNC_F_ASYNC_WAIT_CTX_SET_WAIT_FD               0
# endif

/*
 * ASYNC reason codes.
 */
# define ASYNC_R_FAILED_TO_SET_POOL                       101
# define ASYNC_R_FAILED_TO_SWAP_CONTEXT                   102
# define ASYNC_R_INIT_FAILED                              105
# define ASYNC_R_INVALID_POOL_SIZE                        103

#endif
