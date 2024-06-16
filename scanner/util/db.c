#include <arpa/inet.h>
#include <libbson-1.0/bson.h>
#include <libmongoc-1.0/mongoc.h>
#include <netdb.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "../inc/db.h"
#include "../inc/log.h"

mongoc_client_pool_t *pool = NULL;

bool db_init(char *mongo) {
  mongoc_client_t      *client;
  mongoc_collection_t  *collection;
  mongoc_index_model_t *im;
  bson_error_t          err;
  mongoc_uri_t         *uri = NULL;
  bson_t               *cmd = NULL, *keys = NULL, *opts = NULL, reply;
  bool                  ret = false;

  // init mongoc
  mongoc_init();

  // parse the URI
  uri = mongoc_uri_new_with_error(mongo, &err);
  if (NULL == uri) {
    error("Failed to parse the URI: %s", err.message);
    goto fail;
  }

  // setup the client pool
  pool = mongoc_client_pool_new(uri);
  if (NULL == pool) {
    error("Failed to create the pool");
    goto fail;
  }
  mongoc_client_pool_set_appname(pool, "massacr-scanner");

  // pop and setup the client
  client = mongoc_client_pool_pop(pool);
  if (NULL == client) {
    error("Failed to create the client");
    goto fail;
  }

  collection = mongoc_client_get_collection(client, "massacr", "data");
  if (NULL == collection) {
    error("Failed to access data collection");
    goto fail;
  }

  keys = BCON_NEW("ipv4", BCON_INT32(1));
  opts = BCON_NEW("unique", BCON_BOOL(true));

  im = mongoc_index_model_new(keys, opts);
  if (!mongoc_collection_create_indexes_with_opts(collection, &im, 1, NULL, NULL, &err)) {
    error("Failed to create indexes: %s", err.message);
    goto fail;
  }

  // ping the "admin" database
  cmd = BCON_NEW("ping", BCON_INT32(1));
  if (!mongoc_client_command_simple(client, "admin", cmd, NULL, &reply, &err))
    error("Failed to ping to admin database: %s", err.message);
  else
    ret = true;
  bson_destroy(&reply);

  // cleanup
fail:
  if (NULL != im)
    mongoc_index_model_destroy(im);

  if (NULL != collection)
    mongoc_collection_destroy(collection);

  if (NULL != client)
    mongoc_client_pool_push(pool, client);

  if (NULL != keys)
    bson_destroy(keys);

  if (NULL != opts)
    bson_destroy(opts);

  if (NULL != cmd)
    bson_destroy(cmd);

  if (NULL != uri)
    mongoc_uri_destroy(uri);

  return ret;
}

void db_free() {
  if (NULL != pool)
    mongoc_client_pool_destroy(pool);
  mongoc_cleanup();
}

void db_add(void *ap) {
  db_args_t *args = (db_args_t *)ap;
  char       ipstr[INET_ADDRSTRLEN];
  bool       exists = false;

  mongoc_collection_t  *collection;
  mongoc_client_t      *client;
  bson_array_builder_t *bab;
  const bson_t         *found;
  bson_error_t          err;
  bson_iter_t           iter;
  bson_t               *query = NULL, *doc = NULL, child;

  snprintf(ipstr,
      INET_ADDRSTRLEN,
      "%d.%d.%d.%d",
      (args->ipv4 >> 24) & 255,
      (args->ipv4 >> 16) & 255,
      (args->ipv4 >> 8) & 255,
      (args->ipv4 & 255));

  client     = mongoc_client_pool_pop(pool);
  collection = mongoc_client_get_collection(client, "massacr", "data");
  if (NULL == collection) {
    error("Failed to access data collection");
    goto end;
  }

again:
  query = BCON_NEW("ipv4", BCON_UTF8(ipstr));

  mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);
  if (mongoc_cursor_next(cursor, &found))
    exists = true;
  mongoc_cursor_destroy(cursor);

  bson_destroy(query);
  query = NULL;

  if (!exists) {
    // create new BSON document
    doc = bson_new();

    // "_id": "[new oid]"
    bson_oid_t oid;
    bson_oid_init(&oid, NULL);
    BSON_APPEND_OID(doc, "_id", &oid);

    // "ipv4": "1.2.3.4"
    BSON_APPEND_UTF8(doc, "ipv4", ipstr);

    // "ports": [80]
    BSON_APPEND_ARRAY_BUILDER_BEGIN(doc, "ports", &bab);
    bson_array_builder_append_int32(bab, args->port);
    bson_append_array_builder_end(doc, bab);

    // insert the document
    if (mongoc_collection_insert_one(collection, doc, NULL, NULL, &err))
      goto end;

    if (err.code != MONGOC_ERROR_DUPLICATE_KEY) {
      error("Failed to update document for %s: %s", ipstr, err.message);
      goto end;
    }

    bson_destroy(doc);
    doc = NULL;

    goto again;
  }

  // find the existing document's "_id"
  bson_iter_init_find(&iter, found, "_id");
  if (!BSON_ITER_HOLDS_OID(&iter)) {
    error("Failed to update existing document for %s: OID not found");
    goto end;
  }

  // create the query with "_id": "[found oid]"
  const bson_oid_t *oid = bson_iter_oid(&iter);
  query                 = BCON_NEW("_id", BCON_OID(oid));

  // create the update document with "$push": { "ports": 22 }
  doc = bson_new();
  BSON_APPEND_DOCUMENT_BEGIN(doc, "$addToSet", &child);
  BSON_APPEND_INT32(&child, "ports", args->port);
  bson_append_document_end(doc, &child);

  // update the document
  if (!mongoc_collection_update_one(collection, query, doc, NULL, NULL, &err))
    error("Failed to update document for %s: %s", ipstr, err.message);

end:
  if (NULL != client)
    mongoc_client_pool_push(pool, client);

  if (NULL != collection)
    mongoc_collection_destroy(collection);

  if (NULL != query)
    bson_destroy(query);

  if (NULL != doc)
    bson_destroy(doc);

  free(args);
}
