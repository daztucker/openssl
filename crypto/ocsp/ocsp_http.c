/*
 * Copyright 2001-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <openssl/ocsp.h>
#include <openssl/http.h>
#include "../http/http_local.h"

#ifndef OPENSSL_NO_OCSP

OSSL_HTTP_REQ_CTX *OCSP_sendreq_new(BIO *io, const char *path,
                                    const OCSP_REQUEST *req, int maxline)
{
    OSSL_HTTP_REQ_CTX *rctx = OSSL_HTTP_REQ_CTX_new(io, io, maxline);

    if (rctx == NULL)
        return NULL;

    if (!OSSL_HTTP_REQ_CTX_set_request_line(rctx, 1 /* POST */, NULL, NULL, path))
        goto err;

    if (!OSSL_HTTP_REQ_CTX_set_expected(rctx,
                                        NULL /* content_type */, 1 /* asn1 */,
                                        0 /* timeout */, 0 /* keep_alive */))
        goto err;
    if (req != NULL
        && !OSSL_HTTP_REQ_CTX_set1_req(rctx, "application/ocsp-request",
                                       ASN1_ITEM_rptr(OCSP_REQUEST),
                                       (ASN1_VALUE *)req))
        goto err;

    return rctx;

 err:
    OSSL_HTTP_REQ_CTX_free(rctx);
    return NULL;
}

OCSP_RESPONSE *OCSP_sendreq_bio(BIO *b, const char *path, OCSP_REQUEST *req)
{
    OCSP_RESPONSE *resp = NULL;
    OSSL_HTTP_REQ_CTX *ctx;
    BIO *mem;

    ctx = OCSP_sendreq_new(b, path, req, -1 /* default max resp line length */);
    if (ctx == NULL)
        return NULL;
    mem = OSSL_HTTP_REQ_CTX_exchange(ctx);
    resp = (OCSP_RESPONSE *)
        ASN1_item_d2i_bio(ASN1_ITEM_rptr(OCSP_RESPONSE), mem, NULL);
    BIO_free(mem);

    /* this indirectly calls ERR_clear_error(): */
    OSSL_HTTP_REQ_CTX_free(ctx);

    return resp;
}
#endif /* !defined(OPENSSL_NO_OCSP) */
