#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ts_hashmap.h"

//i don't know if it's ok to do this or not
pthread_mutex_t *mutexGet;
pthread_mutex_t *mutexPut;
pthread_mutex_t *mutexDel;
pthread_mutex_t *mutexOpPlus;

/**
 * Creates a new thread-safe hashmap. 
 *
 * @param capacity initial capacity of the hashmap.
 * @return a pointer to a new thread-safe hashmap.
 */
ts_hashmap_t *initmap(int capacity) {
  pthread_mutex_init(mutexGet, NULL);
  pthread_mutex_init(mutexPut, NULL);
  pthread_mutex_init(mutexDel, NULL);
  pthread_mutex_init(mutexOpPlus, NULL);
  ts_hashmap_t* hashMap = (ts_hashmap_t*)malloc(sizeof(ts_hashmap_t)*capacity);
  ts_entry_t **table = (ts_entry_t**)malloc(sizeof(ts_entry_t*)*capacity);
  hashMap->capacity = capacity;
  hashMap->table = table;
  hashMap->size = 0;
  for(int i=0;i<capacity;i++){
    hashMap->table[i] = NULL;
  }
  return hashMap;
}

/**
 * Obtains the value associated with the given key.
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int get(ts_hashmap_t *map, int key) {
  pthread_mutex_lock(mutexGet);
  int returnVal = 0;
  for(int i=0;i<map->capacity; i++){
    ts_entry_t *current = map->table[i];
    if(current == NULL){
      returnVal = INT_MAX;
    } else if (current->key == key){
      returnVal = current->value;
    } else {
      returnVal = INT_MAX;
    }
  }
  pthread_mutex_lock(mutexOpPlus);
  map->numOps++;
  pthread_mutex_unlock(mutexOpPlus);

  pthread_mutex_unlock(mutexGet);
  return returnVal;
}

/**
 * Associates a value associated with a given key.
 * @param map a pointer to the map
 * @param key a key
 * @param value a value
 * @return old associated value, or INT_MAX if the key was new
 */
int put(ts_hashmap_t *map, int key, int value) {
  pthread_mutex_lock(mutexPut);
  int oldVal = get(map, key);
  int returnVal = 0;
  if(oldVal == INT_MAX){
      returnVal = INT_MAX;
  }else{
      returnVal = oldVal;
  }
  ts_entry_t* newEntry = (ts_entry_t*)malloc(sizeof(ts_entry_t));
  newEntry->key = key;
  newEntry->value = value;
  pthread_mutex_lock(mutexOpPlus);
  map->numOps++;
  pthread_mutex_unlock(mutexOpPlus);

  pthread_mutex_unlock(mutexPut);
  return returnVal;
}

/**
 * Removes an entry in the map
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int del(ts_hashmap_t *map, int key) {
  pthread_mutex_lock(mutexDel);
  int returnVal = 0;
  for(int i=0;i<map->capacity; i++){
    ts_entry_t *current = map->table[i];
    ts_entry_t *nextEntry = current->next;
    if(nextEntry == NULL){
      returnVal = INT_MAX;
    } else if (nextEntry->key == key){
      returnVal = nextEntry->value;
      current->next = nextEntry->next; //move the pointer to the next entry 
      free(nextEntry);
    } else {
      returnVal = INT_MAX;
    }
  }
  pthread_mutex_lock(mutexOpPlus);
  map->numOps++;
  pthread_mutex_unlock(mutexOpPlus);

  pthread_mutex_unlock(mutexDel);
  return returnVal;
}


/**
 * Prints the contents of the map (given)
 */
void printmap(ts_hashmap_t *map) {
  for (int i = 0; i < map->capacity; i++) {
    printf("[%d] -> ", i);
    ts_entry_t *entry = map->table[i];
    while (entry != NULL) {
      printf("(%d,%d)", entry->key, entry->value);
      if (entry->next != NULL)
        printf(" -> ");
      entry = entry->next;
    }
    printf("\n");
  }
}

/**
 * Free up the space allocated for hashmap
 * @param map a pointer to the map
 */
void freeMap(ts_hashmap_t *map) {
  for (int i = 0; i < map->capacity; i++) {
    ts_entry_t* current = map->table[i];
    while (current != NULL) {
        ts_entry_t* next = current->next;
        free(current);
        current = next;
    }
}
free(map->table);
free(map);
pthread_mutex_destroy(mutexGet);
pthread_mutex_destroy(mutexPut);
pthread_mutex_destroy(mutexDel);
pthread_mutex_destroy(mutexOpPlus);
}