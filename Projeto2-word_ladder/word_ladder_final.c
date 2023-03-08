//
// AED, November 2022 (Tomás Oliveira e Silva)
//
// Second practical assignement (speed run)
//
// Place your student numbers and names here
//   N.Mec. 107572  Name: Gonçalo Lopes
//   N.Mec. 108317  Name: Miguel Miragaia
//   N.Mec.   Name: 
//
// Do as much as you can
//   1) MANDATORY: complete the hash table code
//      *) hash_table_create  ##feito n
//      *) hash_table_grow    ##feito n
//      *) hash_table_free    ##feito n
//      *) find_word          ##feito d
//      +) add code to get some statistical data about the hash table     ##feito n/d
//   2) HIGHLY RECOMMENDED: build the graph (including union-find data) -- use the similar_words function...
//      *) find_representative  ##feito n/d
//      *) add_edge   ##feito n/d
//   3) RECOMMENDED: implement breadth-first search in the graph
//      *) breadh_first_search
//   4) RECOMMENDED: list all words belonginh to a connected component
//      *) breadh_first_search
//      *) list_connected_component   ##feito d/n
//   5) RECOMMENDED: find the shortest path between to words
//      *) breadh_first_search
//      *) path_finder
//      *) test the smallest path from bem to mal
//         [ 0] bem
//         [ 1] tem
//         [ 2] teu
//         [ 3] meu
//         [ 4] mau
//         [ 5] mal
//      *) find other interesting word ladders
//   6) OPTIONAL: compute the diameter of a connected component and list the longest word chain
//      *) breadh_first_search
//      *) connected_component_diameter
//   7) OPTIONAL: print some statistics about the graph
//      *) graph_info
//   8) OPTIONAL: test for memory leaks
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

//
// static configuration
//

#define _max_word_size_  32


//
// data structures (SUGGESTION --- you may do it in a different way)
//

typedef struct adjacency_node_s  adjacency_node_t;
typedef struct hash_table_node_s hash_table_node_t;
typedef struct hash_table_s      hash_table_t;

struct adjacency_node_s
{
  adjacency_node_t *next;            // link to th enext adjacency list node
  hash_table_node_t *vertex;         // the other vertex
};

struct hash_table_node_s
{
  // the hash table data
  char word[_max_word_size_];        // the word(key)
  hash_table_node_t *next;           // next hash table linked list node
  // the vertex data
  adjacency_node_t *head;            // head of the linked list of adjancency edges
  int visited;                       // visited status (while not in use, keep it at 0)
  hash_table_node_t *previous;       // breadth-first search parent
  // the union find data
  hash_table_node_t *representative; // the representative of the connected component this vertex belongs to
  int number_of_vertices;            // number of vertices of the conected component (only correct for the representative of each connected component)
  int number_of_edges;                // number of edges of the conected component (only correct for the representative of each connected component)
        
};

struct hash_table_s
{
  unsigned int hash_table_size;      // the size of the hash table array
  unsigned int number_of_entries;    // the number of entries in the hash table
  unsigned int number_of_edges;
  unsigned int number_of_collisions;
  unsigned int number_of_components;
  unsigned int number_of_edge_nodes;      // number of edges (for information purposes only)
  hash_table_node_t **heads;         // the heads of the linked lists
};

typedef struct deque_s
{
  void **items;
  unsigned int hi;
  unsigned int low;
  unsigned int size;
  unsigned int maxsize;
  int full;
} deque_t;

static deque_t *create_deque(int maxsize)
{
  deque_t *deque = malloc(sizeof(deque_t));
  if(deque == NULL)
	{
		fprintf(stderr,"create_deque: out of memory\n");
		exit(1);
	}
  deque->items = (void **)malloc(sizeof(void *) * maxsize);
  if(deque->items == NULL){
		fprintf(stderr,"create_deque->circular_array: out of memory\n");
		free(deque);
		exit(1);
	}
  deque->maxsize = maxsize;
  deque->hi = 0;
  deque->low = 0;
  deque->full = 0;
  deque->size = 0;
  return deque;
}

static void put_hi(deque_t *deque, void *item)
{
	assert(deque->size < deque->maxsize);
	deque->items[deque->hi] = item;
	deque->hi = (deque->hi + 1) % deque->maxsize;
	deque->size++;
}

static void *get_low(deque_t *deque)
{
	assert(deque->size > 0);
	void *ret = deque->items[deque->low];
	deque->low = (deque->low + 1) % deque->maxsize;
	deque->size--;
	return ret;
}

void delete_deque(deque_t *deque)
{
  free(deque->items);
  free(deque);
} 


//
// allocation and deallocation of linked list nodes (done)
//

static adjacency_node_t *allocate_adjacency_node(void)
{
  adjacency_node_t *node;

  node = (adjacency_node_t *)malloc(sizeof(adjacency_node_t));
  if(node == NULL)
  {
    fprintf(stderr,"allocate_adjacency_node: out of memory\n");
    exit(1);
  }
  return node;
}

static void free_adjacency_node(adjacency_node_t *node)
{
  free(node);
}

static hash_table_node_t *allocate_hash_table_node(void)
{
  hash_table_node_t *node;

  node = (hash_table_node_t *)malloc(sizeof(hash_table_node_t));
  if(node == NULL)
  {
    fprintf(stderr,"allocate_hash_table_node: out of memory\n");
    exit(1);
  }
  return node;
}

static void free_hash_table_node(hash_table_node_t *node)
{
  free(node);
}

//
// hash table stuff (mostly to be done)
//

unsigned int crc32(const char *str)
{
  static unsigned int table[256];
  unsigned int crc;

  if(table[1] == 0u) // do we need to initialize the table[] array?
  {
    unsigned int i,j;

    for(i = 0u;i < 256u;i++)
      for(table[i] = i,j = 0u;j < 8u;j++)
        if(table[i] & 1u)
          table[i] = (table[i] >> 1) ^ 0xAED00022u; // "magic" constant
        else
          table[i] >>= 1;
  }
  crc = 0xAED02022u; // initial value (chosen arbitrarily)
  while(*str != '\0')
    crc = (crc >> 8) ^ table[crc & 0xFFu] ^ ((unsigned int)*str++ << 24);
  return crc;
}



static hash_table_t *hash_table_create(void)
{
  hash_table_t *hash_table;
  unsigned int i;


  hash_table = (hash_table_t *)malloc(sizeof(hash_table_t));
  if(hash_table == NULL)
  {
    fprintf(stderr,"create_hash_table: out of memory\n");
    exit(1);
  }
  //
  // complete this
  //
  hash_table->hash_table_size = 1000;
  hash_table->number_of_entries = 0u;
  hash_table->number_of_components = 0u;
  hash_table->number_of_collisions = 0u;
  hash_table->number_of_edges =0u;
  hash_table->number_of_edge_nodes = 0u;
  if( ( hash_table->heads = (hash_table_node_t **) malloc( hash_table->hash_table_size * sizeof(hash_table_node_t*) ) ) == NULL ) {
		fprintf(stderr,"create_hash_table: out of memory\n");
    exit(1);
	}
	for( i = 0; i < hash_table->hash_table_size; i++ ) {
		hash_table->heads[i] = NULL;
	}

	return hash_table;
}

static void hash_table_grow(hash_table_t *hash_table)
{
  //
  // complete this
  //
  unsigned int new_index;
  unsigned int new_size= 2 * (hash_table->hash_table_size + 1);
  hash_table_node_t **new_head;
  hash_table_node_t *temp_next;
  if (hash_table->number_of_collisions > 0 && (hash_table->hash_table_size / hash_table->number_of_collisions) < 5)
	{
    new_head = malloc(new_size *sizeof(hash_table_node_t*));
    if( new_head == NULL ) {
      fprintf(stderr,"grow_hash_table: out of memory\n");
      exit(1);
    }
    hash_table->number_of_collisions = 0u;
    for(unsigned int i=0; i < hash_table->hash_table_size;i++){
      hash_table_node_t *tmp = hash_table->heads[i];
      while(tmp!=NULL){
        new_index = crc32(tmp->word)%new_size;
        temp_next = tmp->next;
        tmp->next = new_head[new_index];
        if (tmp->next){
          hash_table->number_of_collisions++;
        }
        new_head[new_index] = tmp;
        tmp = temp_next;
      }
    }
    free(hash_table->heads);
    hash_table->heads=new_head;
    hash_table->hash_table_size=new_size;
  }
}


static void hash_table_free(hash_table_t *hash_table)
{
  hash_table_node_t *temp_next;
  
  for(unsigned int i=0; i < hash_table->hash_table_size;i++){
    hash_table_node_t *tmp = hash_table->heads[i];
    while(tmp!=NULL){
      temp_next = tmp->next;
      //quando houver adjacencias tenho de as libertar aqui. as adjacencias estao no campo head ( adjacencie_node_t) ciclo while percorrer a lista de adjacencias e libertar cada um dos nós
      adjacency_node_t *adj_tmp = tmp->head;
      while (adj_tmp != NULL) {
        adjacency_node_t *adj_temp_next = adj_tmp->next;
        free_adjacency_node(adj_tmp);
        adj_tmp = adj_temp_next;
      }
      free(tmp);
      tmp = temp_next;
    }
  }
  free(hash_table->heads);
  free(hash_table);
}

//função que cria um novo nó
static hash_table_node_t *create_node(const char *word)
{
	hash_table_node_t *node = allocate_hash_table_node();
  node->next = NULL;
  node->head = NULL;
  node->visited = 0;
	node->representative = node;
	node->previous = NULL;
	node->number_of_vertices = 1;
	node->number_of_edges = 0;
	strcpy(node->word, word);
	return node;
}


static hash_table_node_t *find_word(hash_table_t *hash_table,const char *word,int insert_if_not_found)
{
  hash_table_node_t *node;
  unsigned int i;
 
  
  i = crc32(word) % hash_table->hash_table_size;
  node = hash_table->heads[i];
	while (node)
	{
		if (strcmp(node->word, word) == 0)
			return node;
		node = node->next;
	}
	if (insert_if_not_found)
	{
		node = create_node(word);
		if (hash_table->heads[i]){
      hash_table->number_of_collisions++;
    }
		node->next = hash_table->heads[i];
		hash_table->heads[i] = node;
    hash_table->number_of_components++;
		hash_table->number_of_entries++;
		hash_table_grow(hash_table);
	}
  return node;
}

static void hash_table_stats(hash_table_t *hash_table)
{
	printf("Number of entries: %u\nCollisions: %u\nHash Table size: %u\n",
			hash_table->number_of_entries,
      hash_table->number_of_collisions,
			hash_table->hash_table_size);
}



//
// add edges to the word ladder graph (mostly do be done)
//

static hash_table_node_t *find_representative(hash_table_node_t *node)
{
  hash_table_node_t *representative,*next_node;
  representative = node;
  while (representative != representative->representative) {
    if (representative == NULL){
      fprintf(stderr,"find_representative: representative not found\n");
      exit(1);
    }
    representative = representative->representative;
  }

  next_node = node;
  while (next_node != representative) {
    node = next_node->representative;
    next_node->representative = representative;
    next_node = node;
  }
  return representative;
}

static void insert_edge(hash_table_t *hash_table, hash_table_node_t *from, hash_table_node_t *to)
{
	adjacency_node_t *link;

	link = allocate_adjacency_node();
	link->vertex = to;
	link->next = from->head;
	from->head = link;
	hash_table->number_of_edge_nodes++;
}

static void add_edge(hash_table_t *hash_table,hash_table_node_t *from,const char *word)
{
  hash_table_node_t *to,*from_representative,*to_representative;
  adjacency_node_t *link;

  to = find_word(hash_table,word,0);
  if(to == NULL){
	    return;
  }
  link = from->head;
  while (link != NULL && link->vertex != to) {
    link = link->next;
  }
  if (link)
		return;
  link = to->head;
  while (link != NULL && link->vertex != from) {
    link = link->next;
  }
  if (link)
		return;
  hash_table->number_of_edges++;

  insert_edge(hash_table, from, to);
	insert_edge(hash_table, to, from);

  from_representative = find_representative(from);
	to_representative = find_representative(to);
  if (from_representative != to_representative) {
    unsigned int vert_sum = from_representative->number_of_vertices + to_representative->number_of_vertices;
    unsigned int edge_sum = from_representative->number_of_edges + to_representative->number_of_edges;
    hash_table_node_t *new_rep;

    if (to_representative->number_of_vertices > from_representative->number_of_vertices) {
        new_rep = from_representative;
        to_representative->representative = new_rep;
    } 
    else {
        new_rep = to_representative;
        from_representative->representative = new_rep;
    }
    new_rep->number_of_vertices = vert_sum;
    new_rep->number_of_edges = edge_sum;
    hash_table->number_of_components--;
  }
}


//
// generates a list of similar words and calls the function add_edge for each one (done)
//
// man utf8 for details on the uft8 encoding
//

static void break_utf8_string(const char *word,int *individual_characters)
{
  int byte0,byte1;

  while(*word != '\0')
  {
    byte0 = (int)(*(word++)) & 0xFF;
    if(byte0 < 0x80)
      *(individual_characters++) = byte0; // plain ASCII character
    else
    {
      byte1 = (int)(*(word++)) & 0xFF;
      if((byte0 & 0b11100000) != 0b11000000 || (byte1 & 0b11000000) != 0b10000000)
      {
        fprintf(stderr,"break_utf8_string: unexpected UFT-8 character\n");
        exit(1);
      }
      *(individual_characters++) = ((byte0 & 0b00011111) << 6) | (byte1 & 0b00111111); // utf8 -> unicode
    }
  }
  *individual_characters = 0; // mark the end!
}

static void make_utf8_string(const int *individual_characters,char word[_max_word_size_])
{
  int code;

  while(*individual_characters != 0)
  {
    code = *(individual_characters++);
    if(code < 0x80)
      *(word++) = (char)code;
    else if(code < (1 << 11))
    { // unicode -> utf8
      *(word++) = 0b11000000 | (code >> 6);
      *(word++) = 0b10000000 | (code & 0b00111111);
    }
    else
    {
      fprintf(stderr,"make_utf8_string: unexpected UFT-8 character\n");
      exit(1);
    }
  }
  *word = '\0';  // mark the end
}

static void similar_words(hash_table_t *hash_table,hash_table_node_t *from)
{
  static const int valid_characters[] =
  { // unicode!
    0x2D,                                                                       // -
    0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,           // A B C D E F G H I J K L M
    0x4E,0x4F,0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,           // N O P Q R S T U V W X Y Z
    0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,           // a b c d e f g h i j k l m
    0x6E,0x6F,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,           // n o p q r s t u v w x y z
    0xC1,0xC2,0xC9,0xCD,0xD3,0xDA,                                              // Á Â É Í Ó Ú
    0xE0,0xE1,0xE2,0xE3,0xE7,0xE8,0xE9,0xEA,0xED,0xEE,0xF3,0xF4,0xF5,0xFA,0xFC, // à á â ã ç è é ê í î ó ô õ ú ü
    0
  };
  int i,j,k,individual_characters[_max_word_size_];
  char new_word[2 * _max_word_size_];

  break_utf8_string(from->word,individual_characters);
  for(i = 0;individual_characters[i] != 0;i++)
  {
    k = individual_characters[i];
    for(j = 0;valid_characters[j] != 0;j++)
    {
      individual_characters[i] = valid_characters[j];
      make_utf8_string(individual_characters,new_word);
      // avoid duplicate cases
      if(strcmp(new_word,from->word) > 0)
        add_edge(hash_table,from,new_word);
    }
    individual_characters[i] = k;
  }
}


//
// breadth-first search (to be done)
//
// returns the number of vertices visited; if the last one is goal, following the previous links gives the shortest path between goal and origin
//

static unsigned int breadh_first_search(unsigned int maximum_number_of_vertices,hash_table_node_t **list_of_vertices,hash_table_node_t *origin,hash_table_node_t *goal)
{	
	unsigned int		num_visited;
	hash_table_node_t	*node;
	adjacency_node_t	*neighbour;
	deque_t 		*deque;

	deque = create_deque(maximum_number_of_vertices);

	num_visited = 0;
	put_hi(deque, origin);
	while (deque->size > 0 && num_visited < maximum_number_of_vertices)
	{
		node = get_low(deque);
		node->visited = 1;
		if (list_of_vertices)
			list_of_vertices[num_visited] = node;
		num_visited++;
		if (node == goal)
			break;
		for(neighbour = node->head; neighbour ; neighbour = neighbour->next)
		{
			if (!neighbour->vertex->visited)
			{
				neighbour->vertex->visited = 1;
				neighbour->vertex->previous = node;
				put_hi(deque, neighbour->vertex);	
			}
		}
	}
	delete_deque(deque);
	return num_visited;
}

void mark_all_vertices(hash_table_t *hash_table) {
    for (unsigned int i = 0; i < hash_table->hash_table_size; i++) {
        hash_table_node_t *node = hash_table->heads[i];
        while (node != NULL) {
            node->visited = 0;
            node = node->next;
        }
    }
}

//
// list all vertices belonging to a connected component (complete this)
//

static void list_connected_component(hash_table_t *hash_table,const char *word)
{
  //
  // complete this
  //
	hash_table_node_t	*origin, *representative;
	hash_table_node_t	**list_of_vertices;
	unsigned int		list_length, i;

	origin = find_word(hash_table, word, 0);
	if (!origin)
	{
		printf("\nThe word: %s doesn't exist\n", word);
		return;
	}

	mark_all_vertices(hash_table);
	representative = find_representative(origin);
	list_of_vertices = malloc(representative->number_of_vertices * sizeof(hash_table_node_t *));
  if (!list_of_vertices) {
    fprintf(stderr, "list_connected_component: out of memory\n");
    exit(1);
  }
	list_length = breadh_first_search(representative->number_of_vertices, list_of_vertices, origin, NULL);
	for (i=0; i < list_length; i++){
    printf("%s\n", list_of_vertices[i]->word);
  }
	free(list_of_vertices);
}

//
// compute the diameter of a connected component (optional)
//

static int largest_diameter;
static hash_table_node_t **largest_diameter_example;

static int connected_component_diameter(hash_table_node_t *node)
{
  int diameter;

  //
  // complete this
  //
  return diameter;
}


//
// find the shortest path from a given word to another given word (to be done)
//

void path_finder(hash_table_t *hash_table, const char *from_word, const char *to_word) {
    hash_table_node_t *from = find_word(hash_table, from_word, 0);
    hash_table_node_t *to = find_word(hash_table, to_word, 0);

    if (!from) {
        fprintf(stderr, "\nWord not found: %s\n", from_word);
        return;
    }
    if (!to) {
        fprintf(stderr, "\nWord not found: %s\n", to_word);
        return;
    }

    mark_all_vertices(hash_table);
    size_t list_len = breadh_first_search(find_representative(to)->number_of_vertices, NULL, to, from);
    if (list_len == 0) {
        fprintf(stderr, "Words are not connected\n");
    } else {
        size_t i = 0;
        while (from && from != to) {
            printf("  [%zu] %s\n", i++, from->word);
            from = from->previous;
        }
        printf("  [%zu] %s\n", i++, from->word);
    }
}



//
// some graph information (optional)
//

static void graph_info(hash_table_t *hash_table)
{
  printf("\nNodes: %u\nEdges: %u\nComponents: %u\n",
			hash_table->number_of_entries,
      hash_table->number_of_edges,
			hash_table->number_of_components);
}


//
// main program
//

int main(int argc,char **argv)
{
  char word[100],from[100],to[100];
  hash_table_t *hash_table;
  hash_table_node_t *node;
  unsigned int i;
  int command;
  FILE *fp;

  // initialize hash table
  hash_table = hash_table_create();
  // read words
  fp = fopen((argc < 2) ? "wordlist-big-latest.txt" : argv[1],"rb");
  if(fp == NULL)
  {
    fprintf(stderr,"main: unable to open the words file\n");
    exit(1);
  }
  while(fscanf(fp,"%99s",word) == 1)
    (void)find_word(hash_table,word,1);
  fclose(fp);
  // find all similar words
  for(i = 0u;i < hash_table->hash_table_size;i++)
    for(node = hash_table->heads[i];node != NULL;node = node->next)
      similar_words(hash_table,node);
  graph_info(hash_table);
  // ask what to do
  for(;;)
  {
    fprintf(stderr,"Your wish is my command:\n");
    fprintf(stderr,"  1 WORD       (list the connected component WORD belongs to)\n");
    fprintf(stderr,"  2 FROM TO    (list the shortest path from FROM to TO)\n");
    fprintf(stderr,"  3            (list hash table stats)\n");
    fprintf(stderr,"  4            (list graph info)\n");
    fprintf(stderr,"  0            (terminate)\n");
    
    fprintf(stderr,"> ");
    if(scanf("%99s",word) != 1)
      break;
    command = atoi(word);
    if(command == 1)
    {
      if(scanf("%99s",word) != 1)
        break;
      list_connected_component(hash_table,word);
    }
    else if(command == 2)
    {
      if(scanf("%99s",from) != 1)
        break;
      if(scanf("%99s",to) != 1)
        break;
      path_finder(hash_table,from,to);
    }
    else if(command == 0)
      break;
    else if(command == 3)
			hash_table_stats(hash_table);		
		else if(command == 4)
			graph_info(hash_table);
  }
  // clean up
  hash_table_free(hash_table);
  return 0;
}