#pragma once

#include <libpq-fe.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    PGconn **connections;
    int *in_use;
    int size;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    char *conninfo;
} DBPool;

DBPool *db_pool_create(const char *conninfo, int size);
PGconn *db_pool_acquire(DBPool *pool);
PGresult *db_query(const char *query)
void db_pool_release(DBPool *pool, PGconn *conn);
void db_pool_destroy(DBPool *pool);

#ifdef DB_IMPLEMENTATION
DBPool *db_pool_create(const char *conninfo, int size) {
    DBPool *pool = malloc(sizeof(DBPool));
    pool->size = size;
    pool->conninfo = strdup(conninfo);

    pool->connections = malloc(sizeof(PGconn *) * size);
    pool->in_use = malloc(sizeof(int) * size);

    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->cond, NULL);

    for (int i = 0; i < size; i++) {
        pool->connections[i] = PQconnectdb(conninfo);
        pool->in_use[i] = 0;

        if (PQstatus(pool->connections[i]) != CONNECTION_OK) {
            fprintf(stderr, "Connection %d failed: %s\n",
                    i, PQerrorMessage(pool->connections[i]));
        }
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

PGresult *db_query(const char *query) {
    PGconn *conn = db_pool_acquire(global_pool);

    PGresult *res = PQexec(conn, query);

    db_pool_release(global_pool, conn);
    return res;
}

void db_pool_release(DBPool *pool, PGconn *conn) {
    pthread_mutex_lock(&pool->lock);

    for (int i = 0; i < pool->size; i++) {
        if (pool->connections[i] == conn) {
            pool->in_use[i] = 0;
            break;
        }
    }

    pthread_cond_signal(&pool->cond);
    pthread_mutex_unlock(&pool->lock);
}

void db_pool_destroy(DBPool *pool) {
    for (int i = 0; i < pool->size; i++) {
        PQfinish(pool->connections[i]);
    }

    free(pool->connections);
    free(pool->in_use);
    free(pool->conninfo);

    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->cond);

    free(pool);
}
#endif
