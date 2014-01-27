/********************** begin file crypto.h **********************/

/* 
** SQLCipher
** crypto.h developed by Stephen Lombardo (Zetetic LLC) 
** sjlombardo at zetetic dot net
** http://zetetic.net
** 
** Copyright (c) 2008, ZETETIC LLC
** All rights reserved.
** 
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the ZETETIC LLC nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
** 
** THIS SOFTWARE IS PROVIDED BY ZETETIC LLC ''AS IS'' AND ANY
** EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL ZETETIC LLC BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**  
*/
/* BEGIN CRYPTO */
#ifdef SQLITE_HAS_CODEC
#ifndef CRYPTO_H
#define CRYPTO_H

#define FILE_HEADER_SZ 16

#ifndef CIPHER
#define CIPHER "aes-256-cbc"
#endif

#define CIPHER_DECRYPT 0
#define CIPHER_ENCRYPT 1

#define CIPHER_READ_CTX 0
#define CIPHER_WRITE_CTX 1
#define CIPHER_READWRITE_CTX 2

#ifndef PBKDF2_ITER
#define PBKDF2_ITER 4000
#endif

#ifndef DEFAULT_USE_HMAC
#define DEFAULT_USE_HMAC 1
#endif

/* by default, sqlcipher will use a reduced number of iterations to generate
   the HMAC key / or transform a raw cipher key 
   */
#ifndef FAST_PBKDF2_ITER
#define FAST_PBKDF2_ITER 2
#endif

/* this if a fixed random array that will be xor'd with the database salt to ensure that the
   salt passed to the HMAC key derivation function is not the same as that used to derive
   the encryption key. This can be overridden at compile time but it will make the resulting
   binary incompatible with the default builds when using HMAC. A future version of SQLcipher
   will likely allow this to be defined at runtime via pragma */ 
#ifndef HMAC_SALT_MASK
#define HMAC_SALT_MASK 0x3a
#endif

#ifdef CODEC_DEBUG
#define CODEC_TRACE(X)  {printf X;fflush(stdout);}
#else
#define CODEC_TRACE(X)
#endif


/* extensions defined in pragma.c */ 
   
void sqlite3pager_get_codec(Pager *pPager, void **ctx);
int sqlite3pager_is_mj_pgno(Pager *pPager, Pgno pgno);
sqlite3_file *sqlite3Pager_get_fd(Pager *pPager);
void sqlite3pager_sqlite3PagerSetCodec(
  Pager *pPager,
  void *(*xCodec)(void*,void*,Pgno,int),
  void (*xCodecSizeChng)(void*,int,int),
  void (*xCodecFree)(void*),
  void *pCodec
);
/* end extensions defined in pragma.c */
 
/*
**  Simple shared routines for converting hex char strings to binary data
 */
static int cipher_hex2int(char c) {
  return (c>='0' && c<='9') ? (c)-'0' :
         (c>='A' && c<='F') ? (c)-'A'+10 :
         (c>='a' && c<='f') ? (c)-'a'+10 : 0;
}

static void cipher_hex2bin(const char *hex, int sz, unsigned char *out){
  int i;
  for(i = 0; i < sz; i += 2){
    out[i/2] = (cipher_hex2int(hex[i])<<4) | cipher_hex2int(hex[i+1]);
  }
}

/* extensions defined in crypto_impl.c */

typedef struct codec_ctx codec_ctx;

/* utility functions */
int sqlcipher_memcmp(const unsigned char *a0, const unsigned char *a1, int len);
int sqlcipher_pseudorandom(void *, int);
void sqlcipher_free(void *, int);

/* activation and initialization */
void sqlcipher_activate();
int sqlcipher_codec_ctx_init(codec_ctx **, Db *, Pager *, sqlite3_file *, const void *, int);
void sqlcipher_codec_ctx_free(codec_ctx **);
int sqlcipher_codec_key_derive(codec_ctx *);
int sqlcipher_codec_key_copy(codec_ctx *, int);

/* page cipher implementation */
int sqlcipher_page_cipher(codec_ctx *, int, Pgno, int, int, unsigned char *, unsigned char *);

/* context setters & getters */
void sqlcipher_codec_ctx_set_error(codec_ctx *, int);

int sqlcipher_codec_ctx_set_pass(codec_ctx *, const void *, int, int);
void sqlcipher_codec_get_pass(codec_ctx *, void **zKey, int *nKey);

int sqlcipher_codec_ctx_set_pagesize(codec_ctx *, int);
int sqlcipher_codec_ctx_get_pagesize(codec_ctx *);
int sqlcipher_codec_ctx_get_reservesize(codec_ctx *);

int sqlcipher_codec_ctx_set_kdf_iter(codec_ctx *, int, int);
void* sqlcipher_codec_ctx_get_kdf_salt(codec_ctx *ctx);

int sqlcipher_codec_ctx_set_fast_kdf_iter(codec_ctx *, int, int);

int sqlcipher_codec_ctx_set_cipher(codec_ctx *, const char *, int);

void* sqlcipher_codec_ctx_get_data(codec_ctx *);

void sqlcipher_exportFunc(sqlite3_context *, int, sqlite3_value **);

int sqlcipher_codec_ctx_set_use_hmac(codec_ctx *ctx, int use);
/* end extensions defined in crypto_impl.c */

#endif
#endif
/* END CRYPTO */
/********************** end file crypto.h **********************/

/********************** begin file crypto.c **********************/

/* 
** SQLCipher
** crypto.c developed by Stephen Lombardo (Zetetic LLC) 
** sjlombardo at zetetic dot net
** http://zetetic.net
** 
** Copyright (c) 2009, ZETETIC LLC
** All rights reserved.
** 
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the ZETETIC LLC nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
** 
** THIS SOFTWARE IS PROVIDED BY ZETETIC LLC ''AS IS'' AND ANY
** EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL ZETETIC LLC BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**  
*/
/* BEGIN CRYPTO */
#ifdef SQLITE_HAS_CODEC

#include <assert.h>
//#include "sqliteInt.h"
//#include "btreeInt.h"
//#include "crypto.h"

int codec_set_kdf_iter(sqlite3* db, int nDb, int kdf_iter, int for_ctx) {
  struct Db *pDb = &db->aDb[nDb];
  CODEC_TRACE(("codec_set_kdf_iter: entered db=%d nDb=%d kdf_iter=%d for_ctx=%d\n", db, nDb, kdf_iter, for_ctx));

  if(pDb->pBt) {
    codec_ctx *ctx;
    sqlite3pager_get_codec(pDb->pBt->pBt->pPager, (void **) &ctx);
    if(ctx) return sqlcipher_codec_ctx_set_kdf_iter(ctx, kdf_iter, for_ctx);
  }
  return SQLITE_ERROR;
}

int codec_set_fast_kdf_iter(sqlite3* db, int nDb, int kdf_iter, int for_ctx) {
  struct Db *pDb = &db->aDb[nDb];
  CODEC_TRACE(("codec_set_kdf_iter: entered db=%d nDb=%d kdf_iter=%d for_ctx=%d\n", db, nDb, kdf_iter, for_ctx));

  if(pDb->pBt) {
    codec_ctx *ctx;
    sqlite3pager_get_codec(pDb->pBt->pBt->pPager, (void **) &ctx);
    if(ctx) return sqlcipher_codec_ctx_set_fast_kdf_iter(ctx, kdf_iter, for_ctx);
  }
  return SQLITE_ERROR;
}

static int codec_set_btree_to_codec_pagesize(sqlite3 *db, Db *pDb, codec_ctx *ctx) {
  int rc, page_sz, reserve_sz; 

  page_sz = sqlcipher_codec_ctx_get_pagesize(ctx);
  reserve_sz = sqlcipher_codec_ctx_get_reservesize(ctx);

  sqlite3_mutex_enter(db->mutex);
  db->nextPagesize = page_sz; 
  pDb->pBt->pBt->pageSizeFixed = 0; 
  CODEC_TRACE(("codec_set_btree_to_codec_pagesize: sqlite3BtreeSetPageSize() size=%d reserve=%d\n", page_sz, reserve_sz));
  rc = sqlite3BtreeSetPageSize(pDb->pBt, page_sz, reserve_sz, 0);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

int codec_set_use_hmac(sqlite3* db, int nDb, int use) {
  struct Db *pDb = &db->aDb[nDb];

  CODEC_TRACE(("codec_set_use_hmac: entered db=%d nDb=%d use=%d\n", db, nDb, use));

  if(pDb->pBt) {
    int rc;
    codec_ctx *ctx;
    sqlite3pager_get_codec(pDb->pBt->pBt->pPager, (void **) &ctx);
    if(ctx) {
      rc = sqlcipher_codec_ctx_set_use_hmac(ctx, use);
      if(rc != SQLITE_OK) return rc;
      /* since the use of hmac has changed, the page size may also change */
      /* Note: before forcing the page size we need to force pageSizeFixed to 0, else  
             sqliteBtreeSetPageSize will block the change  */
      return codec_set_btree_to_codec_pagesize(db, pDb, ctx);
    }
  }
  return SQLITE_ERROR;
}

int codec_set_page_size(sqlite3* db, int nDb, int size) {
  struct Db *pDb = &db->aDb[nDb];
  CODEC_TRACE(("codec_set_page_size: entered db=%d nDb=%d size=%d\n", db, nDb, size));

  if(pDb->pBt) {
    int rc;
    codec_ctx *ctx;
    sqlite3pager_get_codec(pDb->pBt->pBt->pPager, (void **) &ctx);

    if(ctx) {
      rc = sqlcipher_codec_ctx_set_pagesize(ctx, size);
      if(rc != SQLITE_OK) return rc;
      return codec_set_btree_to_codec_pagesize(db, pDb, ctx);
    }
  }
  return SQLITE_ERROR;
}

/**
  * 
  * when for_ctx == 0 then it will change for read
  * when for_ctx == 1 then it will change for write
  * when for_ctx == 2 then it will change for both
  */
int codec_set_cipher_name(sqlite3* db, int nDb, const char *cipher_name, int for_ctx) {
  struct Db *pDb = &db->aDb[nDb];
  CODEC_TRACE(("codec_set_cipher_name: entered db=%d nDb=%d cipher_name=%s for_ctx=%d\n", db, nDb, cipher_name, for_ctx));

  if(pDb->pBt) {
    codec_ctx *ctx;
    sqlite3pager_get_codec(pDb->pBt->pBt->pPager, (void **) &ctx);
    if(ctx) return sqlcipher_codec_ctx_set_cipher(ctx, cipher_name, for_ctx);
  }
  return SQLITE_ERROR;
}

int codec_set_pass_key(sqlite3* db, int nDb, const void *zKey, int nKey, int for_ctx) {
  struct Db *pDb = &db->aDb[nDb];
  CODEC_TRACE(("codec_set_pass_key: entered db=%d nDb=%d cipher_name=%s nKey=%d for_ctx=%d\n", db, nDb, zKey, nKey, for_ctx));
  if(pDb->pBt) {
    codec_ctx *ctx;
    sqlite3pager_get_codec(pDb->pBt->pBt->pPager, (void **) &ctx);
    if(ctx) return sqlcipher_codec_ctx_set_pass(ctx, zKey, nKey, for_ctx);
  }
  return SQLITE_ERROR;
} 

/*
 * sqlite3Codec can be called in multiple modes.
 * encrypt mode - expected to return a pointer to the 
 *   encrypted data without altering pData.
 * decrypt mode - expected to return a pointer to pData, with
 *   the data decrypted in the input buffer
 */
void* sqlite3Codec(void *iCtx, void *data, Pgno pgno, int mode) {
  codec_ctx *ctx = (codec_ctx *) iCtx;
  int offset = 0, rc = 0;
  int page_sz = sqlcipher_codec_ctx_get_pagesize(ctx); 
  unsigned char *pData = (unsigned char *) data;
  void *buffer = sqlcipher_codec_ctx_get_data(ctx);
  void *kdf_salt = sqlcipher_codec_ctx_get_kdf_salt(ctx);
  CODEC_TRACE(("sqlite3Codec: entered pgno=%d, mode=%d, page_sz=%d\n", pgno, mode, page_sz));

  /* call to derive keys if not present yet */
  if((rc = sqlcipher_codec_key_derive(ctx)) != SQLITE_OK) {
   sqlcipher_codec_ctx_set_error(ctx, rc); 
   return NULL;
  }

  if(pgno == 1) offset = FILE_HEADER_SZ; /* adjust starting pointers in data page for header offset on first page*/

  CODEC_TRACE(("sqlite3Codec: switch mode=%d offset=%d\n",  mode, offset));
  switch(mode) {
    case 0: /* decrypt */
    case 2:
    case 3:
      if(pgno == 1) memcpy(buffer, SQLITE_FILE_HEADER, FILE_HEADER_SZ); /* copy file header to the first 16 bytes of the page */ 
      rc = sqlcipher_page_cipher(ctx, CIPHER_READ_CTX, pgno, CIPHER_DECRYPT, page_sz - offset, pData + offset, (unsigned char*)buffer + offset);
      if(rc != SQLITE_OK) sqlcipher_codec_ctx_set_error(ctx, rc);
      memcpy(pData, buffer, page_sz); /* copy buffer data back to pData and return */
      return pData;
      break;
    case 6: /* encrypt */
      if(pgno == 1) memcpy(buffer, kdf_salt, FILE_HEADER_SZ); /* copy salt to output buffer */ 
      rc = sqlcipher_page_cipher(ctx, CIPHER_WRITE_CTX, pgno, CIPHER_ENCRYPT, page_sz - offset, pData + offset, (unsigned char*)buffer + offset);
      if(rc != SQLITE_OK) sqlcipher_codec_ctx_set_error(ctx, rc);
      return buffer; /* return persistent buffer data, pData remains intact */
      break;
    case 7:
      if(pgno == 1) memcpy(buffer, kdf_salt, FILE_HEADER_SZ); /* copy salt to output buffer */ 
      rc = sqlcipher_page_cipher(ctx, CIPHER_READ_CTX, pgno, CIPHER_ENCRYPT, page_sz - offset, pData + offset, (unsigned char*)buffer + offset);
      if(rc != SQLITE_OK) sqlcipher_codec_ctx_set_error(ctx, rc);
      return buffer; /* return persistent buffer data, pData remains intact */
      break;
    default:
      return pData;
      break;
  }
}

void sqlite3FreeCodecArg(void *pCodecArg) {
  codec_ctx *ctx = (codec_ctx *) pCodecArg;
  if(pCodecArg == NULL) return;
  sqlcipher_codec_ctx_free(&ctx); // wipe and free allocated memory for the context 
}

int sqlite3CodecAttach(sqlite3* db, int nDb, const void *zKey, int nKey) {
  struct Db *pDb = &db->aDb[nDb];

  CODEC_TRACE(("sqlite3CodecAttach: entered nDb=%d zKey=%s, nKey=%d\n", nDb, zKey, nKey));

  sqlcipher_activate();

  if(nKey && zKey && pDb->pBt) {
    int rc;
    Pager *pPager = pDb->pBt->pBt->pPager;
    sqlite3_file *fd = sqlite3Pager_get_fd(pPager);
    codec_ctx *ctx;

    /* point the internal codec argument against the contet to be prepared */
    rc = sqlcipher_codec_ctx_init(&ctx, pDb, pDb->pBt->pBt->pPager, fd, zKey, nKey); 

    sqlite3pager_sqlite3PagerSetCodec(sqlite3BtreePager(pDb->pBt), sqlite3Codec, NULL, sqlite3FreeCodecArg, (void *) ctx);

    codec_set_btree_to_codec_pagesize(db, pDb, ctx);

    /* if fd is null, then this is an in-memory database and
       we dont' want to overwrite the AutoVacuum settings
       if not null, then set to the default */
    sqlite3_mutex_enter(db->mutex);
    if(fd != NULL) { 
      sqlite3BtreeSetAutoVacuum(pDb->pBt, SQLITE_DEFAULT_AUTOVACUUM);
    }
    sqlite3_mutex_leave(db->mutex);
  }
  return SQLITE_OK;
}

void sqlite3_activate_see(const char* in) {
  /* do nothing, security enhancements are always active */
}

int sqlite3_key(sqlite3 *db, const void *pKey, int nKey) {
  CODEC_TRACE(("sqlite3_key: entered db=%d pKey=%s nKey=%d\n", db, pKey, nKey));
  /* attach key if db and pKey are not null and nKey is > 0 */
  if(db && pKey && nKey) {
    sqlite3CodecAttach(db, 0, pKey, nKey); // operate only on the main db 
    return SQLITE_OK;
  }
  return SQLITE_ERROR;
}

/* sqlite3_rekey 
** Given a database, this will reencrypt the database using a new key.
** There is only one possible modes of operation - to encrypt a database
** that is already encrpyted. If the database is not already encrypted
** this should do nothing
** The proposed logic for this function follows:
** 1. Determine if the database is already encryptped
** 2. If there is NOT already a key present do nothing
** 3. If there is a key present, re-encrypt the database with the new key
*/
int sqlite3_rekey(sqlite3 *db, const void *pKey, int nKey) {
  CODEC_TRACE(("sqlite3_rekey: entered db=%d pKey=%s, nKey=%d\n", db, pKey, nKey));
  sqlcipher_activate();
  if(db && pKey && nKey) {
    struct Db *pDb = &db->aDb[0];
    CODEC_TRACE(("sqlite3_rekey: database pDb=%d\n", pDb));
    if(pDb->pBt) {
      codec_ctx *ctx;
      int rc, page_count;
      Pgno pgno;
      PgHdr *page;
      Pager *pPager = pDb->pBt->pBt->pPager;

      sqlite3pager_get_codec(pDb->pBt->pBt->pPager, (void **) &ctx);
     
      if(ctx == NULL) { 
        /* there was no codec attached to this database, so this should do nothing! */ 
        CODEC_TRACE(("sqlite3_rekey: no codec attached to db, exiting\n"));
        return SQLITE_OK;
      }

      sqlite3_mutex_enter(db->mutex);

      codec_set_pass_key(db, 0, pKey, nKey, CIPHER_WRITE_CTX);
    
      /* do stuff here to rewrite the database 
      ** 1. Create a transaction on the database
      ** 2. Iterate through each page, reading it and then writing it.
      ** 3. If that goes ok then commit and put ctx->rekey into ctx->key
      **    note: don't deallocate rekey since it may be used in a subsequent iteration 
      */
      rc = sqlite3BtreeBeginTrans(pDb->pBt, 1); /* begin write transaction */
      sqlite3PagerPagecount(pPager, &page_count);
      for(pgno = 1; rc == SQLITE_OK && pgno <= (u32) page_count; pgno++) { /* pgno's start at 1 see pager.c:pagerAcquire */
        if(!sqlite3pager_is_mj_pgno(pPager, pgno)) { /* skip this page (see pager.c:pagerAcquire for reasoning) */
          rc = sqlite3PagerGet(pPager, pgno, &page);
          if(rc == SQLITE_OK) { /* write page see pager_incr_changecounter for example */
            rc = sqlite3PagerWrite(page);
            //printf("sqlite3PagerWrite(%d)\n", pgno);
            if(rc == SQLITE_OK) {
              sqlite3PagerUnref(page);
            } 
          } 
        } 
      }

      /* if commit was successful commit and copy the rekey data to current key, else rollback to release locks */
      if(rc == SQLITE_OK) { 
        CODEC_TRACE(("sqlite3_rekey: committing\n"));
        rc = sqlite3BtreeCommit(pDb->pBt); 
        sqlcipher_codec_key_copy(ctx, CIPHER_WRITE_CTX);
      } else {
        CODEC_TRACE(("sqlite3_rekey: rollback\n"));
        sqlite3BtreeRollback(pDb->pBt);
      }

      sqlite3_mutex_leave(db->mutex);
    }
    return SQLITE_OK;
  }
  return SQLITE_ERROR;
}

void sqlite3CodecGetKey(sqlite3* db, int nDb, void **zKey, int *nKey) {
  struct Db *pDb = &db->aDb[nDb];
  CODEC_TRACE(("sqlite3CodecGetKey: entered db=%d, nDb=%d\n", db, nDb));
  
  if( pDb->pBt ) {
    codec_ctx *ctx;
    sqlite3pager_get_codec(pDb->pBt->pBt->pPager, (void **) &ctx);

    if(ctx) { /* if the codec has an attached codec_context user the raw key data */
      sqlcipher_codec_get_pass(ctx, zKey, nKey);
    } else {
      *zKey = NULL;
      *nKey = 0;
    }
  }
}


/* END CRYPTO */
#endif
/********************** end file crypto.c **********************/


/********************** begin file crypto_impl.c ***************/
/* 
** SQLCipher
** crypto_impl.c developed by Stephen Lombardo (Zetetic LLC) 
** sjlombardo at zetetic dot net
** http://zetetic.net
** 
** Copyright (c) 2011, ZETETIC LLC
** All rights reserved.
** 
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the ZETETIC LLC nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
** 
** THIS SOFTWARE IS PROVIDED BY ZETETIC LLC ''AS IS'' AND ANY
** EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL ZETETIC LLC BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**  
*/
/* BEGIN CRYPTO */
#ifdef SQLITE_HAS_CODEC

#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
//#include "sqliteInt.h"
//#include "btreeInt.h"
//#include "crypto.h"
#ifndef OMIT_MEMLOCK
#if defined(__unix__) || defined(__APPLE__) 
#include <sys/mman.h>
#elif defined(_WIN32)
# include <windows.h>
#endif
#endif

/* the default implementation of SQLCipher uses a cipher_ctx
   to keep track of read / write state separately. The following
   struct and associated functions are defined here */
typedef struct {
  int derive_key;
  EVP_CIPHER *evp_cipher;
  EVP_CIPHER_CTX ectx;
  HMAC_CTX hctx;
  int kdf_iter;
  int fast_kdf_iter;
  int key_sz;
  int iv_sz;
  int block_sz;
  int pass_sz;
  int reserve_sz;
  int hmac_sz;
  int use_hmac;
  unsigned char *key;
  unsigned char *hmac_key;
  char *pass;
} cipher_ctx;

void sqlcipher_cipher_ctx_free(cipher_ctx **);
int sqlcipher_cipher_ctx_cmp(cipher_ctx *, cipher_ctx *);
int sqlcipher_cipher_ctx_copy(cipher_ctx *, cipher_ctx *);
int sqlcipher_cipher_ctx_init(cipher_ctx **);
int sqlcipher_cipher_ctx_set_pass(cipher_ctx *, const void *, int);
int sqlcipher_cipher_ctx_key_derive(codec_ctx *, cipher_ctx *);

/* prototype for pager HMAC function */
int sqlcipher_page_hmac(cipher_ctx *, Pgno, unsigned char *, int, unsigned char *);

struct codec_ctx {
  int kdf_salt_sz;
  int page_sz;
  unsigned char *kdf_salt;
  unsigned char *hmac_kdf_salt;
  unsigned char *buffer;
  Btree *pBt;
  cipher_ctx *read_ctx;
  cipher_ctx *write_ctx;
};

void sqlcipher_activate() {
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MASTER));
  if(EVP_get_cipherbyname(CIPHER) == NULL) {
    OpenSSL_add_all_algorithms();
  } 
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MASTER));
}

/* fixed time memory comparison routine */
int sqlcipher_memcmp(const unsigned char *a0, const unsigned char *a1, int len) {
  int i = 0, noMatch = 0;

  for(i = 0; i < len; i++) {
    noMatch = (noMatch || (a0[i] != a1[i]));
  }
  
  return noMatch;
}

/* generate a defined number of pseudorandom bytes */
int sqlcipher_random (void *buffer, int length) {
  return RAND_bytes((unsigned char *)buffer, length);
}

/**
  * Free and wipe memory. Uses SQLites internal sqlite3_free so that memory
  * can be countend and memory leak detection works in the tet suite. 
  * If ptr is not null memory will be freed. 
  * If sz is greater than zero, the memory will be overwritten with zero before it is freed
  * If sz is > 0, and not compiled with OMIT_MEMLOCK, system will attempt to unlock the
  * memory segment so it can be paged
  */
void sqlcipher_free(void *ptr, int sz) {
  if(ptr) {
    if(sz > 0) {
      memset(ptr, 0, sz);
#ifndef OMIT_MEMLOCK
#if defined(__unix__) || defined(__APPLE__) 
      munlock(ptr, sz);
#elif defined(_WIN32)
      VirtualUnlock(ptr, sz);
#endif
#endif
    }
    sqlite3_free(ptr);
  }
}

/**
  * allocate memory. Uses sqlite's internall malloc wrapper so memory can be 
  * reference counted and leak detection works. Unless compiled with OMIT_MEMLOCK
  * attempts to lock the memory pages so sensitive information won't be swapped
  */
void* sqlcipher_malloc(int sz) {
  void *ptr = sqlite3Malloc(sz);
#ifndef OMIT_MEMLOCK
  if(ptr) {
#if defined(__unix__) || defined(__APPLE__) 
    mlock(ptr, sz);
#elif defined(_WIN32)
    VirtualLock(ptr, sz);
#endif
  }
#endif
  return ptr;
}


/**
  * Initialize a a new cipher_ctx struct. This function will allocate memory
  * for the cipher context and for the key
  * 
  * returns SQLITE_OK if initialization was successful
  * returns SQLITE_NOMEM if an error occured allocating memory
  */
int sqlcipher_cipher_ctx_init(cipher_ctx **iCtx) {
  cipher_ctx *ctx;
  *iCtx = (cipher_ctx *) sqlcipher_malloc(sizeof(cipher_ctx));
  ctx = *iCtx;
  if(ctx == NULL) return SQLITE_NOMEM;
  memset(ctx, 0, sizeof(cipher_ctx)); 
  ctx->key = (unsigned char *) sqlcipher_malloc(EVP_MAX_KEY_LENGTH);
  ctx->hmac_key = (unsigned char *) sqlcipher_malloc(EVP_MAX_KEY_LENGTH);
  if(ctx->key == NULL) return SQLITE_NOMEM;
  if(ctx->hmac_key == NULL) return SQLITE_NOMEM;
  return SQLITE_OK;
}

/**
  * Free and wipe memory associated with a cipher_ctx
  */
void sqlcipher_cipher_ctx_free(cipher_ctx **iCtx) {
  cipher_ctx *ctx = *iCtx;
  CODEC_TRACE(("cipher_ctx_free: entered iCtx=%d\n", iCtx));
  sqlcipher_free(ctx->key, ctx->key_sz);
  sqlcipher_free(ctx->hmac_key, ctx->key_sz);
  sqlcipher_free(ctx->pass, ctx->pass_sz);
  sqlcipher_free(ctx, sizeof(cipher_ctx)); 
}

/**
  * Compare one cipher_ctx to another.
  *
  * returns 0 if all the parameters (except the derived key data) are the same
  * returns 1 otherwise
  */
int sqlcipher_cipher_ctx_cmp(cipher_ctx *c1, cipher_ctx *c2) {
  CODEC_TRACE(("sqlcipher_cipher_ctx_cmp: entered c1=%d c2=%d\n", c1, c2));

  if(
    c1->evp_cipher == c2->evp_cipher
    && c1->iv_sz == c2->iv_sz
    && c1->kdf_iter == c2->kdf_iter
    && c1->fast_kdf_iter == c2->fast_kdf_iter
    && c1->key_sz == c2->key_sz
    && c1->pass_sz == c2->pass_sz
    && (
      c1->pass == c2->pass
      || !sqlcipher_memcmp((const unsigned char*)c1->pass,
                           (const unsigned char*)c2->pass,
                           c1->pass_sz)
    ) 
  ) return 0;
  return 1;
}

/**
  * Copy one cipher_ctx to another. For instance, assuming that read_ctx is a 
  * fully initialized context, you could copy it to write_ctx and all yet data
  * and pass information across
  *
  * returns SQLITE_OK if initialization was successful
  * returns SQLITE_NOMEM if an error occured allocating memory
  */
int sqlcipher_cipher_ctx_copy(cipher_ctx *target, cipher_ctx *source) {
  void *key = target->key; 
  void *hmac_key = target->hmac_key; 

  CODEC_TRACE(("sqlcipher_cipher_ctx_copy: entered target=%d, source=%d\n", target, source));
  sqlcipher_free(target->pass, target->pass_sz); 
  memcpy(target, source, sizeof(cipher_ctx));
  
  target->key = key; //restore pointer to previously allocated key data
  memcpy(target->key, source->key, EVP_MAX_KEY_LENGTH);

  target->hmac_key = hmac_key; //restore pointer to previously allocated hmac key data
  memcpy(target->hmac_key, source->hmac_key, EVP_MAX_KEY_LENGTH);

  target->pass = sqlcipher_malloc(source->pass_sz);
  if(target->pass == NULL) return SQLITE_NOMEM;
  memcpy(target->pass, source->pass, source->pass_sz);

  return SQLITE_OK;
}

/**
  * Set the raw password / key data for a cipher context
  * 
  * returns SQLITE_OK if assignment was successfull
  * returns SQLITE_NOMEM if an error occured allocating memory
  * returns SQLITE_ERROR if the key couldn't be set because the pass was null or size was zero
  */
int sqlcipher_cipher_ctx_set_pass(cipher_ctx *ctx, const void *zKey, int nKey) {
  sqlcipher_free(ctx->pass, ctx->pass_sz);
  ctx->pass_sz = nKey;
  if(zKey && nKey) {
    ctx->pass = sqlcipher_malloc(nKey);
    if(ctx->pass == NULL) return SQLITE_NOMEM;
    memcpy(ctx->pass, zKey, nKey);
    return SQLITE_OK;
  }
  return SQLITE_ERROR;
}

int sqlcipher_codec_ctx_set_pass(codec_ctx *ctx, const void *zKey, int nKey, int for_ctx) {
  cipher_ctx *c_ctx = for_ctx ? ctx->write_ctx : ctx->read_ctx;
  int rc;

  if((rc = sqlcipher_cipher_ctx_set_pass(c_ctx, zKey, nKey)) != SQLITE_OK) return rc; 
  c_ctx->derive_key = 1;

  if(for_ctx == 2)
    if((rc = sqlcipher_cipher_ctx_copy( for_ctx ? ctx->read_ctx : ctx->write_ctx, c_ctx)) != SQLITE_OK) 
      return rc; 

  return SQLITE_OK;
} 

int sqlcipher_codec_ctx_set_cipher(codec_ctx *ctx, const char *cipher_name, int for_ctx) {
  cipher_ctx *c_ctx = for_ctx ? ctx->write_ctx : ctx->read_ctx;
  int rc;

  c_ctx->evp_cipher = (EVP_CIPHER *) EVP_get_cipherbyname(cipher_name);
  c_ctx->key_sz = EVP_CIPHER_key_length(c_ctx->evp_cipher);
  c_ctx->iv_sz = EVP_CIPHER_iv_length(c_ctx->evp_cipher);
  c_ctx->block_sz = EVP_CIPHER_block_size(c_ctx->evp_cipher);
  c_ctx->hmac_sz = EVP_MD_size(EVP_sha1());
  c_ctx->derive_key = 1;

  if(for_ctx == 2)
    if((rc = sqlcipher_cipher_ctx_copy( for_ctx ? ctx->read_ctx : ctx->write_ctx, c_ctx)) != SQLITE_OK)
      return rc; 

  return SQLITE_OK;
}

int sqlcipher_codec_ctx_set_kdf_iter(codec_ctx *ctx, int kdf_iter, int for_ctx) {
  cipher_ctx *c_ctx = for_ctx ? ctx->write_ctx : ctx->read_ctx;
  int rc;

  c_ctx->kdf_iter = kdf_iter;
  c_ctx->derive_key = 1;

  if(for_ctx == 2)
    if((rc = sqlcipher_cipher_ctx_copy( for_ctx ? ctx->read_ctx : ctx->write_ctx, c_ctx)) != SQLITE_OK)
      return rc; 

  return SQLITE_OK;
}

int sqlcipher_codec_ctx_set_fast_kdf_iter(codec_ctx *ctx, int fast_kdf_iter, int for_ctx) {
  cipher_ctx *c_ctx = for_ctx ? ctx->write_ctx : ctx->read_ctx;
  int rc;

  c_ctx->fast_kdf_iter = fast_kdf_iter;
  c_ctx->derive_key = 1;

  if(for_ctx == 2)
    if((rc = sqlcipher_cipher_ctx_copy( for_ctx ? ctx->read_ctx : ctx->write_ctx, c_ctx)) != SQLITE_OK)
      return rc; 

  return SQLITE_OK;
}

int sqlcipher_codec_ctx_set_use_hmac(codec_ctx *ctx, int use) {
  int reserve = EVP_MAX_IV_LENGTH; /* base reserve size will be IV only */ 

  if(use) reserve += ctx->read_ctx->hmac_sz; /* if reserve will include hmac, update that size */

  /* calculate the amount of reserve needed in even increments of the cipher block size */

  reserve = ((reserve % ctx->read_ctx->block_sz) == 0) ? reserve :
               ((reserve / ctx->read_ctx->block_sz) + 1) * ctx->read_ctx->block_sz;  

  CODEC_TRACE(("sqlcipher_codec_ctx_set_use_hmac: use=%d block_sz=%d md_size=%d reserve=%d\n", 
                use, ctx->read_ctx->block_sz, ctx->read_ctx->hmac_sz, reserve)); 

  ctx->write_ctx->use_hmac = ctx->read_ctx->use_hmac = use;
  ctx->write_ctx->reserve_sz = ctx->read_ctx->reserve_sz = reserve;

  return SQLITE_OK;
}

void sqlcipher_codec_ctx_set_error(codec_ctx *ctx, int error) {
  ctx->pBt->db->errCode = error;
}

int sqlcipher_codec_ctx_get_pagesize(codec_ctx *ctx) {
  return ctx->page_sz;
}

int sqlcipher_codec_ctx_get_reservesize(codec_ctx *ctx) {
  return ctx->read_ctx->reserve_sz;
}

void* sqlcipher_codec_ctx_get_data(codec_ctx *ctx) {
  return ctx->buffer;
}

void* sqlcipher_codec_ctx_get_kdf_salt(codec_ctx *ctx) {
  return ctx->kdf_salt;
}

void sqlcipher_codec_get_pass(codec_ctx *ctx, void **zKey, int *nKey) {
  *zKey = ctx->read_ctx->pass;
  *nKey = ctx->read_ctx->pass_sz;
}

int sqlcipher_codec_ctx_set_pagesize(codec_ctx *ctx, int size) {
  /* attempt to free the existing page buffer */
  sqlcipher_free(ctx->buffer,ctx->page_sz);
  ctx->page_sz = size;

  /* pre-allocate a page buffer of PageSize bytes. This will
     be used as a persistent buffer for encryption and decryption 
     operations to avoid overhead of multiple memory allocations*/
  ctx->buffer = sqlcipher_malloc(size);
  if(ctx->buffer == NULL) return SQLITE_NOMEM;

  return SQLITE_OK;
}

int sqlcipher_codec_ctx_init(codec_ctx **iCtx, Db *pDb, Pager *pPager, sqlite3_file *fd, const void *zKey, int nKey) {
  int rc;
  codec_ctx *ctx;
  *iCtx = sqlcipher_malloc(sizeof(codec_ctx));
  ctx = *iCtx;

  if(ctx == NULL) return SQLITE_NOMEM;

  memset(ctx, 0, sizeof(codec_ctx)); /* initialize all pointers and values to 0 */
  ctx->pBt = pDb->pBt; /* assign pointer to database btree structure */

  /* allocate space for salt data. Then read the first 16 bytes 
       directly off the database file. This is the salt for the
       key derivation function. If we get a short read allocate
       a new random salt value */
  ctx->kdf_salt_sz = FILE_HEADER_SZ;
  ctx->kdf_salt = sqlcipher_malloc(ctx->kdf_salt_sz);
  if(ctx->kdf_salt == NULL) return SQLITE_NOMEM;

  /* allocate space for separate hmac salt data. We want the
     HMAC derivation salt to be different than the encryption
     key derivation salt */
  ctx->hmac_kdf_salt = sqlcipher_malloc(ctx->kdf_salt_sz);
  if(ctx->hmac_kdf_salt == NULL) return SQLITE_NOMEM;


  /*
     Always overwrite page size and set to the default because the first page of the database
     in encrypted and thus sqlite can't effectively determine the pagesize. this causes an issue in 
     cases where bytes 16 & 17 of the page header are a power of 2 as reported by John Lehman
  */
  if((rc = sqlcipher_codec_ctx_set_pagesize(ctx, SQLITE_DEFAULT_PAGE_SIZE)) != SQLITE_OK) return rc;

  if((rc = sqlcipher_cipher_ctx_init(&ctx->read_ctx)) != SQLITE_OK) return rc; 
  if((rc = sqlcipher_cipher_ctx_init(&ctx->write_ctx)) != SQLITE_OK) return rc; 

  if(fd == NULL || sqlite3OsRead(fd, ctx->kdf_salt, FILE_HEADER_SZ, 0) != SQLITE_OK) {
    /* if unable to read the bytes, generate random salt */
    if(sqlcipher_random(ctx->kdf_salt, FILE_HEADER_SZ) != 1) return SQLITE_ERROR;
  }

  if((rc = sqlcipher_codec_ctx_set_cipher(ctx, CIPHER, 0)) != SQLITE_OK) return rc;
  if((rc = sqlcipher_codec_ctx_set_kdf_iter(ctx, PBKDF2_ITER, 0)) != SQLITE_OK) return rc;
  if((rc = sqlcipher_codec_ctx_set_fast_kdf_iter(ctx, FAST_PBKDF2_ITER, 0)) != SQLITE_OK) return rc;
  if((rc = sqlcipher_codec_ctx_set_pass(ctx, zKey, nKey, 0)) != SQLITE_OK) return rc;

  /* Use HMAC signatures by default. Note that codec_set_use_hmac will implicity call
     codec_set_page_size to set the default */
  if((rc = sqlcipher_codec_ctx_set_use_hmac(ctx, DEFAULT_USE_HMAC)) != SQLITE_OK) return rc;

  if((rc = sqlcipher_cipher_ctx_copy(ctx->write_ctx, ctx->read_ctx)) != SQLITE_OK) return rc;

  return SQLITE_OK;
}

/**
  * Free and wipe memory associated with a cipher_ctx, including the allocated
  * read_ctx and write_ctx.
  */
void sqlcipher_codec_ctx_free(codec_ctx **iCtx) {
  codec_ctx *ctx = *iCtx;
  CODEC_TRACE(("codec_ctx_free: entered iCtx=%d\n", iCtx));
  sqlcipher_free(ctx->kdf_salt, ctx->kdf_salt_sz);
  sqlcipher_free(ctx->hmac_kdf_salt, ctx->kdf_salt_sz);
  sqlcipher_free(ctx->buffer, 0);
  sqlcipher_cipher_ctx_free(&ctx->read_ctx);
  sqlcipher_cipher_ctx_free(&ctx->write_ctx);
  sqlcipher_free(ctx, sizeof(codec_ctx)); 
}

int sqlcipher_page_hmac(cipher_ctx *ctx, Pgno pgno, unsigned char *in, int in_sz, unsigned char *out) {
  HMAC_CTX_init(&ctx->hctx);
  
  HMAC_Init_ex(&ctx->hctx, ctx->hmac_key, ctx->key_sz, EVP_sha1(), NULL);

  /* include the encrypted page data,  initialization vector, and page number in HMAC. This will 
     prevent both tampering with the ciphertext, manipulation of the IV, or resequencing otherwise
     valid pages out of order in a database */ 
  HMAC_Update(&ctx->hctx, in, in_sz);
  HMAC_Update(&ctx->hctx, (const unsigned char*) &pgno, sizeof(Pgno));
  HMAC_Final(&ctx->hctx, out, NULL);
  HMAC_CTX_cleanup(&ctx->hctx);
  return SQLITE_OK; 
}

/*
 * ctx - codec context
 * pgno - page number in database
 * size - size in bytes of input and output buffers
 * mode - 1 to encrypt, 0 to decrypt
 * in - pointer to input bytes
 * out - pouter to output bytes
 */
int sqlcipher_page_cipher(codec_ctx *ctx, int for_ctx, Pgno pgno, int mode, int page_sz, unsigned char *in, unsigned char *out) {
  cipher_ctx *c_ctx = for_ctx ? ctx->write_ctx : ctx->read_ctx;
  unsigned char *iv_in, *iv_out, *hmac_in, *hmac_out, *out_start;
  int tmp_csz, csz, size;

  /* calculate some required positions into various buffers */
  size = page_sz - c_ctx->reserve_sz; /* adjust size to useable size and memset reserve at end of page */
  iv_out = out + size;
  iv_in = in + size;

  /* hmac will be written immediately after the initialization vector. the remainder of the page reserve will contain
     random bytes. note, these pointers are only valid when use_hmac is true */
  hmac_in = in + size + c_ctx->iv_sz; 
  hmac_out = out + size + c_ctx->iv_sz;
  out_start = out; /* note the original position of the output buffer pointer, as out will be rewritten during encryption */

  CODEC_TRACE(("codec_cipher:entered pgno=%d, mode=%d, size=%d\n", pgno, mode, size));

  /* just copy raw data from in to out when key size is 0
   * i.e. during a rekey of a plaintext database */ 
  if(c_ctx->key_sz == 0) {
    memcpy(out, in, size);
    return SQLITE_OK;
  } 

  if(mode == CIPHER_ENCRYPT) {
    /* start at front of the reserve block, write random data to the end */
    if(sqlcipher_random(iv_out, c_ctx->reserve_sz) != 1) return SQLITE_ERROR; 
  } else { /* CIPHER_DECRYPT */
    memcpy(iv_out, iv_in, c_ctx->iv_sz); /* copy the iv from the input to output buffer */
  } 

  if(c_ctx->use_hmac && (mode == CIPHER_DECRYPT)) {
    if(sqlcipher_page_hmac(c_ctx, pgno, in, size + c_ctx->iv_sz, hmac_out) != SQLITE_OK) {
      memset(out, 0, page_sz); 
      CODEC_TRACE(("codec_cipher: hmac operations failed for pgno=%d\n", pgno));
      return SQLITE_ERROR;
    }

    CODEC_TRACE(("codec_cipher: comparing hmac on in=%d out=%d hmac_sz=%d\n", hmac_in, hmac_out, c_ctx->hmac_sz));
    if(sqlcipher_memcmp(hmac_in, hmac_out, c_ctx->hmac_sz) != 0) {
      /* the hmac check failed, which means the data was tampered with or
         corrupted in some way. we will return an error, and zero out the page data
         to force an error */
      memset(out, 0, page_sz); 
      CODEC_TRACE(("codec_cipher: hmac check failed for pgno=%d\n", pgno));
      return SQLITE_ERROR;
    }
  } 

  EVP_CipherInit(&c_ctx->ectx, c_ctx->evp_cipher, NULL, NULL, mode);
  EVP_CIPHER_CTX_set_padding(&c_ctx->ectx, 0);
  EVP_CipherInit(&c_ctx->ectx, NULL, c_ctx->key, iv_out, mode);
  EVP_CipherUpdate(&c_ctx->ectx, out, &tmp_csz, in, size);
  csz = tmp_csz;  
  out += tmp_csz;
  EVP_CipherFinal(&c_ctx->ectx, out, &tmp_csz);
  csz += tmp_csz;
  EVP_CIPHER_CTX_cleanup(&c_ctx->ectx);
  assert(size == csz);

  if(c_ctx->use_hmac && (mode == CIPHER_ENCRYPT)) {
    sqlcipher_page_hmac(c_ctx, pgno, out_start, size + c_ctx->iv_sz, hmac_out); 
  }

  return SQLITE_OK;
}

/**
  * Derive an encryption key for a cipher contex key based on the raw password.
  *
  * If the raw key data is formated as x'hex' and there are exactly enough hex chars to fill
  * the key space (i.e 64 hex chars for a 256 bit key) then the key data will be used directly. 
  * 
  * Otherwise, a key data will be derived using PBKDF2
  * 
  * returns SQLITE_OK if initialization was successful
  * returns SQLITE_ERROR if the key could't be derived (for instance if pass is NULL or pass_sz is 0)
  */
int sqlcipher_cipher_ctx_key_derive(codec_ctx *ctx, cipher_ctx *c_ctx) {
  CODEC_TRACE(("codec_key_derive: entered c_ctx->pass=%s, c_ctx->pass_sz=%d \
                ctx->kdf_salt=%d ctx->kdf_salt_sz=%d c_ctx->kdf_iter=%d \
                ctx->hmac_kdf_salt=%d, c_ctx->fast_kdf_iter=%d c_ctx->key_sz=%d\n", 
                c_ctx->pass, c_ctx->pass_sz, ctx->kdf_salt, ctx->kdf_salt_sz, c_ctx->kdf_iter, 
                ctx->hmac_kdf_salt, c_ctx->fast_kdf_iter, c_ctx->key_sz)); 
                

  if(c_ctx->pass && c_ctx->pass_sz) { // if pass is not null
    if (c_ctx->pass_sz == ((c_ctx->key_sz*2)+3) && sqlite3StrNICmp(c_ctx->pass ,"x'", 2) == 0) { 
      int n = c_ctx->pass_sz - 3; /* adjust for leading x' and tailing ' */
      const char *z = c_ctx->pass + 2; /* adjust lead offset of x' */
      CODEC_TRACE(("codec_key_derive: using raw key from hex\n")); 
      cipher_hex2bin(z, n, c_ctx->key);
    } else { 
      CODEC_TRACE(("codec_key_derive: deriving key using full PBKDF2 with %d iterations\n", c_ctx->kdf_iter)); 
      PKCS5_PBKDF2_HMAC_SHA1( c_ctx->pass, c_ctx->pass_sz, 
                              ctx->kdf_salt, ctx->kdf_salt_sz, 
                              c_ctx->kdf_iter, c_ctx->key_sz, c_ctx->key);
                              
    }

    /* if this context is setup to use hmac checks, generate a seperate and different 
       key for HMAC. In this case, we use the output of the previous KDF as the input to 
       this KDF run. This ensures a distinct but predictable HMAC key. */
    if(c_ctx->use_hmac) {
      int i;

      /* start by copying the kdf key into the hmac salt slot
         then XOR it with the fixed hmac salt defined at compile time
         this ensures that the salt passed in to derive the hmac key, while 
         easy to derive and publically known, is not the same as the salt used 
         to generate the encryption key */ 
      memcpy(ctx->hmac_kdf_salt, ctx->kdf_salt, ctx->kdf_salt_sz);
      for(i = 0; i < ctx->kdf_salt_sz; i++) {
        ctx->hmac_kdf_salt[i] ^= HMAC_SALT_MASK;
      } 

      CODEC_TRACE(("codec_key_derive: deriving hmac key from encryption key using PBKDF2 with %d iterations\n", 
        c_ctx->fast_kdf_iter)); 
      PKCS5_PBKDF2_HMAC_SHA1( (const char*)c_ctx->key, c_ctx->key_sz, 
                              ctx->hmac_kdf_salt, ctx->kdf_salt_sz, 
                              c_ctx->fast_kdf_iter, c_ctx->key_sz, c_ctx->hmac_key); 
    }

    c_ctx->derive_key = 0;
    return SQLITE_OK;
  };
  return SQLITE_ERROR;
}

int sqlcipher_codec_key_derive(codec_ctx *ctx) {
  /* derive key on first use if necessary */
  if(ctx->read_ctx->derive_key) {
    if(sqlcipher_cipher_ctx_key_derive(ctx, ctx->read_ctx) != SQLITE_OK) return SQLITE_ERROR;
  }

  if(ctx->write_ctx->derive_key) {
    if(sqlcipher_cipher_ctx_cmp(ctx->write_ctx, ctx->read_ctx) == 0) {
      // the relevant parameters are the same, just copy read key
      if(sqlcipher_cipher_ctx_copy(ctx->write_ctx, ctx->read_ctx) != SQLITE_OK) return SQLITE_ERROR;
    } else {
      if(sqlcipher_cipher_ctx_key_derive(ctx, ctx->write_ctx) != SQLITE_OK) return SQLITE_ERROR;
    }
  }
  return SQLITE_OK; 
}

int sqlcipher_codec_key_copy(codec_ctx *ctx, int source) {
  if(source == CIPHER_READ_CTX) { 
      return sqlcipher_cipher_ctx_copy(ctx->write_ctx, ctx->read_ctx); 
  } else {
      return sqlcipher_cipher_ctx_copy(ctx->read_ctx, ctx->write_ctx); 
  }
}


#ifndef OMIT_EXPORT

/*
 * Implementation of an "export" function that allows a caller
 * to duplicate the main database to an attached database. This is intended
 * as a conveneince for users who need to:
 * 
 *   1. migrate from an non-encrypted database to an encrypted database
 *   2. move from an encrypted database to a non-encrypted database
 *   3. convert beween the various flavors of encrypted databases.  
 *
 * This implementation is based heavily on the procedure and code used
 * in vacuum.c, but is exposed as a function that allows export to any
 * named attached database.
 */

/*
** Finalize a prepared statement.  If there was an error, store the
** text of the error message in *pzErrMsg.  Return the result code.
** 
** Based on vacuumFinalize from vacuum.c
*/
static int sqlcipher_finalize(sqlite3 *db, sqlite3_stmt *pStmt, char **pzErrMsg){
  int rc;
  rc = sqlite3VdbeFinalize((Vdbe*)pStmt);
  if( rc ){
    sqlite3SetString(pzErrMsg, db, sqlite3_errmsg(db));
  }
  return rc;
}

/*
** Execute zSql on database db. Return an error code.
** 
** Based on execSql from vacuum.c
*/
static int sqlcipher_execSql(sqlite3 *db, char **pzErrMsg, const char *zSql){
  sqlite3_stmt *pStmt;
  VVA_ONLY( int rc; )
  if( !zSql ){
    return SQLITE_NOMEM;
  }
  if( SQLITE_OK!=sqlite3_prepare(db, zSql, -1, &pStmt, 0) ){
    sqlite3SetString(pzErrMsg, db, sqlite3_errmsg(db));
    return sqlite3_errcode(db);
  }
  VVA_ONLY( rc = ) sqlite3_step(pStmt);
  assert( rc!=SQLITE_ROW );
  return sqlcipher_finalize(db, pStmt, pzErrMsg);
}

/*
** Execute zSql on database db. The statement returns exactly
** one column. Execute this as SQL on the same database.
** 
** Based on execExecSql from vacuum.c
*/
static int sqlcipher_execExecSql(sqlite3 *db, char **pzErrMsg, const char *zSql){
  sqlite3_stmt *pStmt;
  int rc;

  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  if( rc!=SQLITE_OK ) return rc;

  while( SQLITE_ROW==sqlite3_step(pStmt) ){
    rc = sqlcipher_execSql(db, pzErrMsg, (char*)sqlite3_column_text(pStmt, 0));
    if( rc!=SQLITE_OK ){
      sqlcipher_finalize(db, pStmt, pzErrMsg);
      return rc;
    }
  }

  return sqlcipher_finalize(db, pStmt, pzErrMsg);
}

/*
 * copy database and schema from the main database to an attached database
 * 
 * Based on sqlite3RunVacuum from vacuum.c
*/
void sqlcipher_exportFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  sqlite3 *db = sqlite3_context_db_handle(context);
  const char* attachedDb = (const char*) sqlite3_value_text(argv[0]);
  int saved_flags;        /* Saved value of the db->flags */
  int saved_nChange;      /* Saved value of db->nChange */
  int saved_nTotalChange; /* Saved value of db->nTotalChange */
  void (*saved_xTrace)(void*,const char*);  /* Saved db->xTrace */
  int rc = SQLITE_OK;     /* Return code from service routines */
  char *zSql = NULL;         /* SQL statements */
  char *pzErrMsg = NULL;
  
  saved_flags = db->flags;
  saved_nChange = db->nChange;
  saved_nTotalChange = db->nTotalChange;
  saved_xTrace = db->xTrace;
  db->flags |= SQLITE_WriteSchema | SQLITE_IgnoreChecks | SQLITE_PreferBuiltin;
  db->flags &= ~(SQLITE_ForeignKeys | SQLITE_ReverseOrder);
  db->xTrace = 0;

  /* Query the schema of the main database. Create a mirror schema
  ** in the temporary database.
  */
  zSql = sqlite3_mprintf(
    "SELECT 'CREATE TABLE %s.' || substr(sql,14) "
    "  FROM sqlite_master WHERE type='table' AND name!='sqlite_sequence'"
    "   AND rootpage>0"
  , attachedDb);
  rc = (zSql == NULL) ? SQLITE_NOMEM : sqlcipher_execExecSql(db, &pzErrMsg, zSql); 
  if( rc!=SQLITE_OK ) goto end_of_export;
  sqlite3_free(zSql);

  zSql = sqlite3_mprintf(
    "SELECT 'CREATE INDEX %s.' || substr(sql,14)"
    "  FROM sqlite_master WHERE sql LIKE 'CREATE INDEX %%' "
  , attachedDb);
  rc = (zSql == NULL) ? SQLITE_NOMEM : sqlcipher_execExecSql(db, &pzErrMsg, zSql); 
  if( rc!=SQLITE_OK ) goto end_of_export;
  sqlite3_free(zSql);

  zSql = sqlite3_mprintf(
    "SELECT 'CREATE UNIQUE INDEX %s.' || substr(sql,21) "
    "  FROM sqlite_master WHERE sql LIKE 'CREATE UNIQUE INDEX %%'"
  , attachedDb);
  rc = (zSql == NULL) ? SQLITE_NOMEM : sqlcipher_execExecSql(db, &pzErrMsg, zSql); 
  if( rc!=SQLITE_OK ) goto end_of_export;
  sqlite3_free(zSql);

  /* Loop through the tables in the main database. For each, do
  ** an "INSERT INTO rekey_db.xxx SELECT * FROM main.xxx;" to copy
  ** the contents to the temporary database.
  */
  zSql = sqlite3_mprintf(
    "SELECT 'INSERT INTO %s.' || quote(name) "
    "|| ' SELECT * FROM main.' || quote(name) || ';'"
    "FROM main.sqlite_master "
    "WHERE type = 'table' AND name!='sqlite_sequence' "
    "  AND rootpage>0"
  , attachedDb);
  rc = (zSql == NULL) ? SQLITE_NOMEM : sqlcipher_execExecSql(db, &pzErrMsg, zSql); 
  if( rc!=SQLITE_OK ) goto end_of_export;
  sqlite3_free(zSql);

  /* Copy over the sequence table
  */
  zSql = sqlite3_mprintf(
    "SELECT 'DELETE FROM %s.' || quote(name) || ';' "
    "FROM %s.sqlite_master WHERE name='sqlite_sequence' "
  , attachedDb, attachedDb);
  rc = (zSql == NULL) ? SQLITE_NOMEM : sqlcipher_execExecSql(db, &pzErrMsg, zSql); 
  if( rc!=SQLITE_OK ) goto end_of_export;
  sqlite3_free(zSql);

  zSql = sqlite3_mprintf(
    "SELECT 'INSERT INTO %s.' || quote(name) "
    "|| ' SELECT * FROM main.' || quote(name) || ';' "
    "FROM %s.sqlite_master WHERE name=='sqlite_sequence';"
  , attachedDb, attachedDb);
  rc = (zSql == NULL) ? SQLITE_NOMEM : sqlcipher_execExecSql(db, &pzErrMsg, zSql); 
  if( rc!=SQLITE_OK ) goto end_of_export;
  sqlite3_free(zSql);

  /* Copy the triggers, views, and virtual tables from the main database
  ** over to the temporary database.  None of these objects has any
  ** associated storage, so all we have to do is copy their entries
  ** from the SQLITE_MASTER table.
  */
  zSql = sqlite3_mprintf(
    "INSERT INTO %s.sqlite_master "
    "  SELECT type, name, tbl_name, rootpage, sql"
    "    FROM main.sqlite_master"
    "   WHERE type='view' OR type='trigger'"
    "      OR (type='table' AND rootpage=0)"
  , attachedDb);
  rc = (zSql == NULL) ? SQLITE_NOMEM : sqlcipher_execSql(db, &pzErrMsg, zSql); 
  if( rc!=SQLITE_OK ) goto end_of_export;
  sqlite3_free(zSql);

  zSql = NULL;
end_of_export:
  db->flags = saved_flags;
  db->nChange = saved_nChange;
  db->nTotalChange = saved_nTotalChange;
  db->xTrace = saved_xTrace;

  sqlite3_free(zSql);

  if(rc) {
    if(pzErrMsg != NULL) {
      sqlite3_result_error(context, pzErrMsg, -1);
      sqlite3DbFree(db, pzErrMsg);
    } else {
      sqlite3_result_error(context, sqlite3ErrStr(rc), -1);
    }
  }
}

#endif
#endif
/********************** end file crypto_impl.c ******************/

/********************** begin file pager.c ***********************/
/* BEGIN CRYPTO */
#ifdef SQLITE_HAS_CODEC
void sqlite3pager_get_codec(Pager *pPager, void **ctx) {
  *ctx = pPager->pCodec;
}

int sqlite3pager_is_mj_pgno(Pager *pPager, Pgno pgno) {
  return (PAGER_MJ_PGNO(pPager) == pgno) ? 1 : 0;
}

sqlite3_file *sqlite3Pager_get_fd(Pager *pPager) {
  return (isOpen(pPager->fd)) ? pPager->fd : NULL;
}

void sqlite3pager_sqlite3PagerSetCodec(
  Pager *pPager,
  void *(*xCodec)(void*,void*,Pgno,int),
  void (*xCodecSizeChng)(void*,int,int),
  void (*xCodecFree)(void*),
  void *pCodec
){
  sqlite3PagerSetCodec(pPager, xCodec, xCodecSizeChng, xCodecFree, pCodec); 
}


#endif
/* END CRYPTO */
/********************** end file pager.c *************************/
