#define MY_DLL_EXPORTS 1

#include "util.h"
#include "irc.h"
#include "module.h"

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

MY_API int get_ssl_fd()
{
    printf("ssl_fd: %d\n", ssl_fd);
    return ssl_fd;
}

MY_API void sslmod_connect()
{
    struct irc_conn *bot = get_bot();

#ifdef _WIN32
    if (SSL_set_fd(ssl, bot->srv_fd) == 0)
#else
    if (SSL_set_fd(ssl, bot->srv_fdi) == 0)
#endif
    {
        eprint("Error: Cannot set SSL file descriptor\n");
    }

    if (SSL_connect(ssl) != 1)
    {
        eprint("Error: Cannot connect to SSL server\n");
    }

#ifdef _WIN32
    bot->ssl_fd = bot->srv_fd;
#else
    bot->ssl_fd = bot->srv_fdi;
#endif
}

MY_API void sslmod_write(char *buf, int len)
{
    int n;
    if ((n = SSL_write(ssl, buf, len)) <= 0)
    {
        eprint("Error: Cannot write to SSL server\n");
    }
} 

MY_API int sslmod_read(char *buf, int len)
{
    int n;
    unsigned long ssl_err;

    if ((n = SSL_read(ssl, buf, len)) <= 0)
    {
        eprint("xbot: error on SSL_read()\n");

        ssl_err = ERR_get_error();
        if (ssl_err)
        {
            eprint("SSL error: %s\n", ERR_error_string(ssl_err, NULL));
        }

        return -1;
    }
    return n;
}

MY_API void sslmod_cleanup()
{
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    ERR_free_strings();
    EVP_cleanup();
}

MY_API void mod_init()
{
    struct irc_conn *bot = get_bot();

    register_module("openssl", "Aaron Blakely", "1.0", "SSL/TLS support using OpenSSL", MOD_FLAG_NO_UNLOAD);
    set_ssl_init(bot, sslmod_init);
    set_ssl_connect(bot, sslmod_connect);
    set_ssl_write(bot, sslmod_write);
    set_ssl_read(bot, sslmod_read);
    set_ssl_cleanup(bot, sslmod_cleanup);

    printf("OpenSSL module loaded\n");
}

MY_API void mod_unload()
{
    // should never be called because of MOD_FLAG_NO_UNLOAD
}
