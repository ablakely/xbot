#define MY_DLL_EXPORTS 1

#include "util.h"
#include "irc.h"
#include "events.h"
#include "module.h"
#include "timers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

int ssl_fd;
SSL *ssl;
SSL_CTX *ctx;

MY_API void sslmod_init(struct irc_conn *bot)
{
    SSL_library_init();
    SSL_load_error_strings();
    ctx = SSL_CTX_new(SSLv23_client_method());
    if (ctx == NULL)
    {
        eprint("Error: Cannot create SSL context\n");
    }

    if (bot->verify_ssl)
    {
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    }
    else
    {
        SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
    }

    if ((ssl = SSL_new(ctx)) == NULL)
    {
        eprint("Error: Cannot create SSL object\n");
    }
}

MY_API void sslmod_connect(struct irc_conn *bot)
{
    if (SSL_set_fd(ssl, fileno(bot->srv_fd)) == 0)
    {
        eprint("Error: Cannot set SSL file descriptor\n");
    }

    if (SSL_connect(ssl) != 1)
    {
        eprint("Error: Cannot connect to SSL server\n");
    }

    ssl_fd = fileno(bot->srv_fd);
}

MY_API void mod_init()
{
    register_module("openssl", "Aaron Blakely", "1.0", "SSL/TLS support using OpenSSL");


}

MY_API void mod_unlaod()
{
    unregister_module("openssl");

}
