#pragma once
#include <libpq-fe.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "macros.h"

typedef struct {
    PGconn        **connections;
    int            *in_use;
    int             size;
    pthread_mutex_t lock;
    pthread_cond_t  cond;
    char           *conninfo;
} DBPool;

typedef struct {
    PGresult *res;
    int       row;
    int       nrows;
} DBRows;

STATIC_LIB_DEF int db_next(DBRows *r) {
    return r->res && (++(r->row) < r->nrows);
}

STATIC_LIB_DEF void db_rows_free(DBRows *r) {
    if (r->res) { PQclear(r->res); r->res = NULL; }
}

STATIC_LIB_DEF int db_col(DBRows *r, const char *name) {
    return PQfnumber(r->res, name);
}

STATIC_LIB_DEF char *db_get_str(DBRows *r, const char *col, size_t len) {
    char out[256];
    int c = db_col(r, col);
    if (c < 0 || PQgetisnull(r->res, r->row, c)) { out[0] = '\0'; return; }
    strncpy(out, PQgetvalue(r->res, r->row, c), len - 1);
    out[len - 1] = '\0';
    return out;
}

STATIC_LIB_DEF char *db_get_strdup(DBRows *r, const char *col) {
    int c = db_col(r, col);
    if (c < 0 || PQgetisnull(r->res, r->row, c)) return NULL;
    return strdup(PQgetvalue(r->res, r->row, c));
}

STATIC_LIB_DEF int    db_get_int   (DBRows *r, const char *col) {
    int c = db_col(r, col);
    return (c < 0 || PQgetisnull(r->res, r->row, c)) ? 0 : atoi(PQgetvalue(r->res, r->row, c));
}

STATIC_LIB_DEF long   db_get_long  (DBRows *r, const char *col) {
    int c = db_col(r, col);
    return (c < 0 || PQgetisnull(r->res, r->row, c)) ? 0L : atol(PQgetvalue(r->res, r->row, c));
}

STATIC_LIB_DEF double db_get_double(DBRows *r, const char *col) {
    int c = db_col(r, col);
    return (c < 0 || PQgetisnull(r->res, r->row, c)) ? 0.0 : atof(PQgetvalue(r->res, r->row, c));
}

DBPool   *db_pool_create (const char *conninfo, int size);
PGconn   *db_pool_acquire(DBPool *pool);
void      db_pool_release(DBPool *pool, PGconn *conn);
void      db_pool_destroy(DBPool *pool);
DBRows    db_query       (DBPool *pool, const char *query);
DBRows    db_queryf      (DBPool *pool, const char *fmt, ...);

#ifdef DB_IMPLEMENTATION
#include <stdarg.h>

DBPool *db_pool_create(const char *conninfo, int size) {
    DBPool *pool = malloc(sizeof(DBPool));
    pool->size = size;
    pool->conninfo = strdup(conninfo);
    pool->connections = malloc(sizeof(PGconn *) * size);
    pool->in_use      = malloc(sizeof(int)      * size);
    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init (&pool->cond, NULL);
    for (int i = 0; i < size; i++) {
        pool->connections[i] = PQconnectdb(conninfo);
        pool->in_use[i]      = 0;
        if (PQstatus(pool->connections[i]) != CONNECTION_OK)
            fprintf(stderr, "Connection %d failed: %s\n", i,
                    PQerrorMessage(pool->connections[i]));
    }
    return pool;
}

PGconn *db_pool_acquire(DBPool *pool) {
    pthread_mutex_lock(&pool->lock);
    while (1) {
        for (int i = 0; i < pool->size; i++) {
            if (!pool->in_use[i]) {
                pool->in_use[i] = 1;
                PGconn *conn = pool->connections[i];
                pthread_mutex_unlock(&pool->lock);
                return conn;
            }
        }
        pthread_cond_wait(&pool->cond, &pool->lock);
    }
}

void db_pool_release(DBPool *pool, PGconn *conn) {
    pthread_mutex_lock(&pool->lock);
    for (int i = 0; i < pool->size; i++) {
        if (pool->connections[i] == conn) { pool->in_use[i] = 0; break; }
    }
    pthread_cond_signal(&pool->cond);
    pthread_mutex_unlock(&pool->lock);
}

void db_pool_destroy(DBPool *pool) {
    for (int i = 0; i < pool->size; i++) PQfinish(pool->connections[i]);
    free(pool->connections);
    free(pool->in_use);
    free(pool->conninfo);
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy (&pool->cond);
    free(pool);
}

DBRows db_query(DBPool *pool, const char *query) {
    PGconn   *conn = db_pool_acquire(pool);
    PGresult *res  = PQexec(conn, query);
    db_pool_release(pool, conn);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Query failed: %s\n", PQresultErrorMessage(res));
        PQclear(res);
        return (DBRows){ NULL, -1, 0 };
    }
    return (DBRows){ res, -1, PQntuples(res) };
}

DBRows db_queryf(DBPool *pool, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int   len = vsnprintf(NULL, 0, fmt, ap) + 1;
    va_end(ap);
    char *buf = malloc(len);
    va_start(ap, fmt);
    vsnprintf(buf, len, fmt, ap);
    va_end(ap);
    DBRows r = db_query(pool, buf);
    free(buf);
    return r;
}

#endif
