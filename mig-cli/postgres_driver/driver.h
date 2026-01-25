#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PG_PROTOCOL_VERSION 196608

typedef struct {
    int socket;
    char *host;
    int port;
    char *database;
    char *user;
    char *password;
} PGConnection;

PGConnection* pg_connect(const char *host, int port, const char *database, const char *user, const char *password);
int pg_exec(PGConnection *conn, const char *sql);
void pg_close(PGConnection *conn);

#ifdef POSTGRES_IMPLEMENTATION
  static int send_message(int sock, char type, const char *data, int len) {
      char header[5];
      header[0] = type;
      *(int32_t*)(header + 1) = htonl(len + 4); 
      
      if (send(sock, header, 5, 0) < 0) return -1;
      if (len > 0 && send(sock, data, len, 0) < 0) return -1;
      return 0;
  }

  static int recv_message(int sock, char *type, char **data, int *len) {
      char header[5];
      
      if (recv(sock, header, 5, MSG_WAITALL) != 5) return -1;
      
      *type = header[0];
      *len = ntohl(*(int32_t*)(header + 1)) - 4;
      
      if (*len > 0) {
          *data = malloc(*len);
          if (recv(sock, *data, *len, MSG_WAITALL) != *len) {
              free(*data);
              return -1;
          }
      } else {
          *data = NULL;
      }
      
      return 0;
  }

  PGConnection* pg_connect(const char *host, int port, const char *database, 
                          const char *user, const char *password) {
      PGConnection *conn = malloc(sizeof(PGConnection));
      struct sockaddr_in server;
      struct hostent *he;
      
      conn->socket = socket(AF_INET, SOCK_STREAM, 0);
      if (conn->socket < 0) {
          free(conn);
          return NULL;
      }
      
      he = gethostbyname(host);
      if (!he) {
          close(conn->socket);
          free(conn);
          return NULL;
      }
      
      server.sin_family = AF_INET;
      server.sin_port = htons(port);
      memcpy(&server.sin_addr, he->h_addr_list[0], he->h_length);
      
      if (connect(conn->socket, (struct sockaddr*)&server, sizeof(server)) < 0) {
          close(conn->socket);
          free(conn);
          return NULL;
      }
      
      char startup[1024];
      int pos = 0;
      *(int32_t*)(startup + pos) = htonl(PG_PROTOCOL_VERSION);
      pos += 4;
      
      pos += sprintf(startup + pos, "user") + 1;
      pos += sprintf(startup + pos, "%s", user) + 1;
      pos += sprintf(startup + pos, "database") + 1;
      pos += sprintf(startup + pos, "%s", database) + 1;
      startup[pos++] = 0;
      
      int32_t msg_len = htonl(pos + 4);
      send(conn->socket, &msg_len, 4, 0);
      send(conn->socket, startup, pos, 0);
      
      char msg_type;
      char *msg_data;
      int msg_len_recv;
      
      while (recv_message(conn->socket, &msg_type, &msg_data, &msg_len_recv) == 0) {
          if (msg_type == 'R') {
              int32_t auth_type = ntohl(*(int32_t*)msg_data);
              
              if (auth_type == 0) {
                  free(msg_data);
                  break;
              } else if (auth_type == 3) {
                  send_message(conn->socket, 'p', password, strlen(password) + 1);
              } else {
                  fprintf(stderr, "Unsupported auth type: %d\n", auth_type);
                  free(msg_data);
                  close(conn->socket);
                  free(conn);
                  return NULL;
              }
              free(msg_data);
          } else if (msg_type == 'E') {
              fprintf(stderr, "Auth error: %s\n", msg_data);
              free(msg_data);
              close(conn->socket);
              free(conn);
              return NULL;
          } else if (msg_type == 'Z') {
              free(msg_data);
              break;
          } else {
              free(msg_data);
          }
      }
      
      conn->host = strdup(host);
      conn->port = port;
      conn->database = strdup(database);
      conn->user = strdup(user);
      conn->password = strdup(password);
      
      return conn;
  }

  int pg_exec(PGConnection *conn, const char *sql) {
      if (send_message(conn->socket, 'Q', sql, strlen(sql) + 1) < 0) {
          return -1;
      }
      
      char msg_type;
      char *msg_data;
      int msg_len;
      
      while (recv_message(conn->socket, &msg_type, &msg_data, &msg_len) == 0) {
          if (msg_type == 'C') {
              printf("Command completed: %s\n", msg_data);
              free(msg_data);
          } else if (msg_type == 'E') {
              fprintf(stderr, "SQL Error: %s\n", msg_data);
              free(msg_data);
              return -1;
          } else if (msg_type == 'Z') {
              free(msg_data);
              break;
          } else {
              free(msg_data); 
          }
      }
      
      return 0;
  }

  void pg_close(PGConnection *conn) {
      if (conn) {
          close(conn->socket);
          free(conn->host);
          free(conn->database);
          free(conn->user);
          free(conn->password);
          free(conn);
      }
  }
#endif
