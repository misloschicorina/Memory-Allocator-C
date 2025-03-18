#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define max 600

typedef struct node {
	void *data;
	struct node *prev;
	struct node *next;
} node;

typedef struct linked_list {
	node *head;
	unsigned int data_size;
	unsigned int size;
} linked_list;

typedef struct segregated_free_lists {
	unsigned int size; //dimensiunea vectorului de liste
	linked_list **array; //pointer catre vectorul de liste
	int bytes_per_list;
} segregated_free_lists;

typedef struct block {
	void *data;
	struct block *prev;
	struct block *next;
	unsigned int *addr;
	unsigned int data_size;
} block; // structura pt blocurile alocate, care au dimensiuni diferite

typedef struct allocated_list {
	block *head;
	unsigned int size;
} allocated_list;

linked_list *create_list(unsigned int data_size)
{
	linked_list *list = calloc(1, sizeof(*list));
	list->data_size = data_size;
	list->head = NULL;
	return list;
}

int cmp_addr(const void *addr1, const void *addr2)
{
	if (*(unsigned int *)addr1 < *(unsigned int *)addr2)
		return -1;
	else if (*(unsigned int *)addr1 > *(unsigned int *)addr2)
		return 1;
	else
		return 0;
}

void add_node(linked_list *list, const void *new_data)
{
	node *new_node = calloc(1, sizeof(*new_node));
	if (!new_node) {
		fprintf(stderr, "Eroare alocare");
		return;
	}
	new_node->data = calloc(1, sizeof(unsigned int));
	if (!new_node->data) {
		fprintf(stderr, "Eroare alocare");
		return;
	}
	memcpy(new_node->data, new_data, sizeof(unsigned int));

	if (!list->head || cmp_addr(new_node->data, list->head->data) < 0) {
		// adaugare la inceputul liatei
		new_node->next = list->head;
		if (list->head)
			list->head->prev = new_node;
		list->head = new_node;
	} else {
		node *curr = list->head;
		while (curr->next && cmp_addr(curr->next->data, new_node->data) < 0)
			curr = curr->next;

		new_node->next = curr->next;
		new_node->prev = curr;
		if (curr->next)
			curr->next->prev = new_node;
		curr->next = new_node;
	}
	list->size++;
}

void remove_nth_node(linked_list *list, unsigned int n)
{
	node *prev, *curr;

	if (!list || !list->head)
		return;

	curr = list->head;
	prev = NULL;
	while (n > 0) {
		prev = curr;
		curr = curr->next;
		--n;
	}

	if (!prev)
		list->head = curr->next;
	else
		prev->next = curr->next;

	list->size--;

	free(curr->data);
	free(curr);
}

segregated_free_lists *init_heap(unsigned int start_addr, int nr_of_lists,
								 int bytes_per_list)
{
	segregated_free_lists *sf_list = calloc(1, sizeof(*sf_list));
	if (!sf_list) {
		fprintf(stderr, "Eroare alocare");
		return NULL;
	}
	sf_list->size = nr_of_lists;
	sf_list->bytes_per_list = bytes_per_list;
	sf_list->array = calloc(nr_of_lists, sizeof(linked_list *));
	if (!sf_list->array) {
		fprintf(stderr, "Eroare alocare");
		return NULL;
	}

	unsigned int curr_addr = start_addr;
	int power = 8;

	for (int i = 0; i < nr_of_lists; i++) {
		sf_list->array[i] = create_list(power); // creez lista de index
		int size = bytes_per_list / sf_list->array[i]->data_size;
		sf_list->array[i]->size = size;
		int j;
		for (j = 0; j < bytes_per_list / sf_list->array[i]->data_size; j++) {
			add_node(sf_list->array[i], &curr_addr);
			curr_addr += power;
			sf_list->array[i]->size = size;
		}
		power *= 2;
	}
	return sf_list;
}

void add_block(allocated_list *list,
			   unsigned int data_size, unsigned int *addr)
{
	block *new_block = calloc(1, sizeof(*new_block));
	if (!new_block) {
		fprintf(stderr, "Eroare alocare");
		return;
	}
	new_block->data_size = data_size;
	new_block->addr = addr;
	new_block->prev = NULL;
	new_block->next = NULL;

	if (!list->head) {
		list->head = new_block;
	} else {
		block *curr = list->head;
		while (curr) {
		// caut sa pun blocul langa cel mai apropiat bloc cu adresa mai mare
			if (*curr->addr > *addr) {
				// pun blocul inainte de curr
				new_block->next = curr;
				new_block->prev = curr->prev;
				if (curr->prev)
					curr->prev->next = new_block;
				else
					list->head = new_block;
				curr->prev = new_block;
				list->size++;
				return;
			}
			curr = curr->next;
		}

		// daca noul block are adresa cea mai mare, il punem la sfarsit
		curr = list->head;
		while (curr->next)
			curr = curr->next;
		curr->next = new_block;
		new_block->prev = curr;
	}

	list->size++;
}

void sort_address(linked_list *list)
{
	node *curr = list->head;

	while (curr && curr->next) {
		node *it = list->head;
		while (it && it->next) {
			if (*(unsigned int *)it->data >
				*(unsigned int *)it->next->data) {
				// interschimb data (adica adresa) dintre it si it->next
				void *temp = it->data;
				it->data = it->next->data;
				it->next->data = temp;
			}
			it = it->next;
		}
		curr = curr->next;
	}
}

// sorteaza listele din vector dupa dimensiune
void sort_lists(segregated_free_lists *sf_list)
{
	int i, j;
	linked_list *list;

	for (i = 1; i < sf_list->size; i++) {
		list = sf_list->array[i];
		j = i - 1;

		// mut listele mai mari catre dreapta
		while (j >= 0 && sf_list->array[j]->data_size > list->data_size) {
			sf_list->array[j + 1] = sf_list->array[j];
			j = j - 1;
		}
		sf_list->array[j + 1] = list;
	}
}

void add_linked_list(segregated_free_lists *sf_list, int pos, int data_size)
{
	// realoc vectorul de liste ca sa fac loc noii liste
	sf_list->array =
	realloc(sf_list->array, (sf_list->size + 1) * sizeof(linked_list *));

	linked_list *new_list = create_list(data_size);

	// deplasez listele deja existente
	for (int i = sf_list->size; i > pos; i--)
		sf_list->array[i] = sf_list->array[i - 1];

	sf_list->array[pos] = new_list;
	sf_list->size++;
}

void malloc_function(segregated_free_lists *sf_list, int nr_bytes,
					 allocated_list *list, int *fragment_nr, int *mallocs_did)
{
	int diff = 0;
	unsigned int new_addr = 0;
	int data_to_remain;
	int found_equal = 0;
	int found_fragment = 0;
	int index_fragment = -1;

	for (int i = 0; i < sf_list->size; i++) {
		for (int j = 0; j < sf_list->array[i]->size; j++) {
			node *curr = sf_list->array[i]->head;
			if (sf_list->array[i]->data_size == nr_bytes) {
				found_equal = 1;
				int *addr = calloc(1, sizeof(unsigned int));
				*addr = *(unsigned int *)curr->data;
				remove_nth_node(sf_list->array[i], 0);
				add_block(list, nr_bytes, addr);
				(*mallocs_did)++;
				break;
			} else if (sf_list->array[i]->data_size >
					   nr_bytes && found_fragment == 0 && found_equal == 0) {
				found_fragment = 1;
				index_fragment = i;
				int *addr = calloc(1, sizeof(unsigned int));
				*addr = *(unsigned int *)curr->data;
				// scot nodul cu sizeul cel mai apropiat > nr_bytes
				remove_nth_node(sf_list->array[i], 0);
				// adaug un bloc de nr_bytes in lista alocata
				add_block(list, nr_bytes, addr);
				diff = sf_list->array[i]->data_size - nr_bytes; // calc dif
				// ca sa adaug un nod in lista din sf_list cu size-ul = diff
				// calc adresa nodului rezultat din fragmentare
				new_addr = *addr + nr_bytes;
				(*mallocs_did)++;
				break;
			}
		}
	}
	if (found_equal == 0 && found_fragment > 0) {
		(*fragment_nr)++; // actualizez numarul de fragmentari
		// caut lista cu dimensiunea exactă diff
		for (int i = 0; i < sf_list->size; i++) {
			if (sf_list->array[i]->data_size == diff) {
				// daca am gasit lista coresp, adaug nod in ea
				add_node(sf_list->array[i], &new_addr);
				return;
			}
		}
		// daca nu am o lista de dimensiunea cautata, creez eu una
		add_linked_list(sf_list, index_fragment, diff);
		add_node(sf_list->array[index_fragment], &new_addr);
		// pastrez listele din array ul de liste sortate dupa data_size ul lor:
		sort_lists(sf_list);
		return;
	}

	if (found_equal == 0 && found_fragment == 0)
		printf("Out of memory\n");
}

void remove_block(allocated_list *list, block *block_to_remove)
{
	if (!list || !block_to_remove)
		return;

	block *curr = list->head;

	while (curr) {
		if (curr == block_to_remove) {
			if (!curr->prev)
				list->head = curr->next;
			else
				curr->prev->next = curr->next;
			if (curr->next)
				curr->next->prev = curr->prev;
			if (curr->data)
				free(curr->data);
			free(curr->addr);
			free(curr);
			list->size--;
			return;
		}
		curr = curr->next;
	}
}

void free_block(allocated_list *list, unsigned int addr,
				segregated_free_lists *sf_list, int *free_nr)
{
	int found = 0;
	int pos = 0;
	if (addr == 0) // daca adresa data e 0x0, nciun efect si se iese
		return;
	int block_to_free_data_size = 0;
	block *curr = list->head;

	while (curr) {
		if (*curr->addr == addr) {
			found = 1;
			block_to_free_data_size = curr->data_size;
			block *next_block = curr->next; // salvez ref la urm bloc
			remove_block(list, curr); // eliberez blocul din lista
			(*free_nr)++;
			curr = next_block; // trec la urm nod
		} else {
			curr = curr->next;
		}
	}
	// acum trebuie sa adaug in sf_list un
	// nod de dimensiunea block_to_free_data_size,
	// intr o lista pe care o am deja sau trebuie s-o creez
	for (int i = 0; i < sf_list->size; i++) {
		if (sf_list->array[i]->data_size == block_to_free_data_size) {
			// daca am gasit lista coresp, adaug nod in ea
			unsigned int *new_addr = &addr;
			add_node(sf_list->array[i], new_addr);
			return;
		}
	}
	// daca nu gasesc o lista cu dimensiunea = block_to_free_data_size,
	// creez una
	if (found == 1) {
		add_linked_list(sf_list, sf_list->size, block_to_free_data_size);
		unsigned int *new_addr = &addr; // adresa blocului eliberat
		add_node(sf_list->array[sf_list->size - 1], new_addr);
		sort_lists(sf_list); // pastrez listele din array sortate
	} else if (found == 0) {
		printf("Invalid free\n");
	}
}

void print_sf_list(segregated_free_lists *sf_list)
{
	for (int i = 0; i < sf_list->size; i++) {
		if (sf_list->array[i]->size != 0) {
			linked_list *curr_list = sf_list->array[i];
			int data_size = curr_list->data_size;
			if (data_size != 0) {
				int free_blocks = sf_list->array[i]->size;
				printf("Blocks with %d bytes - ", data_size);
				printf("%d free block(s) :", free_blocks);

				node *curr_node = curr_list->head;
				while (curr_node) {
					printf(" 0x%x", *(unsigned int *)curr_node->data);
					curr_node = curr_node->next;
				}
				printf("\n");
			}
		}
	}
}

void dump_mem(segregated_free_lists *sf_list, allocated_list *list,
			  int malloc_nr, int free_nr, int fragment_nr, int tot_mem)
{
	int tot_free_blocks = 0;
	int tot_free_mem = 0;
	int tot_allocated_mem = 0;
	for (int i = 0; i < sf_list->size; i++) {
		tot_free_mem += sf_list->array[i]->size * sf_list->array[i]->data_size;
		tot_free_blocks += sf_list->array[i]->size;
	}
	block *curr = list->head;
	for (int i = 0; i < list->size; i++) {
		tot_allocated_mem += curr->data_size;
		curr = curr->next;
	}
	printf("+++++DUMP+++++\n");
	printf("Total memory: %d bytes\n", tot_mem);
	printf("Total allocated memory: %d bytes\n", tot_allocated_mem);
	printf("Total free memory: %d bytes\n", tot_free_mem);
	printf("Free blocks: %d\n", tot_free_blocks);
	printf("Number of allocated blocks: %d\n", list->size);
	printf("Number of malloc calls: %d\n", malloc_nr);
	printf("Number of fragmentations: %d\n", fragment_nr);
	printf("Number of free calls: %d\n", free_nr);

	print_sf_list(sf_list);
	printf("Allocated blocks :");
	curr = list->head;
	for (int i = 0; i < list->size; i++) {
		printf(" (0x%x - ", *(unsigned int *)curr->addr);
		printf("%d)", curr->data_size);
		curr = curr->next;
	}
	printf("\n");
	printf("-----DUMP-----\n");
}

void destroy(segregated_free_lists *sf_list, allocated_list *list)
{
	if (list) {
		block *curr_block = list->head;
		while (curr_block) {
			block *next_block = curr_block->next;
			free(curr_block->data);
			free(curr_block->addr);
			free(curr_block);
			curr_block = next_block;
		}
		free(list);
	}

	if (sf_list) {
		for (int i = 0; i < sf_list->size; ++i) {
			linked_list *curr_list = sf_list->array[i];
			if (curr_list) {
				node *curr_node = curr_list->head;
				while (curr_node) {
					node *next_node = curr_node->next;
					free(curr_node->data);
					free(curr_node);
					curr_node = next_node;
				}
				free(curr_list);
			}
		}
		free(sf_list->array);
		free(sf_list);
	}
}

void write(segregated_free_lists *sf_list, allocated_list *list,
		   unsigned int addr, char *data_string, int nr_bytes, int malloc_nr,
		   int free_nr, int fragment_nr, int tot_mem, int *destroy_write)
{
	int found_addr = 0;

	if (strlen(data_string) < nr_bytes)
		nr_bytes = strlen(data_string);

	int mem_in_list = 0;

	block *temp = list->head;
	block *curr = list->head;
	while (curr) {
		if (addr == *(unsigned int *)curr->addr) {
			found_addr = 1;
			mem_in_list += curr->data_size;
			while (curr->next) {
				if (*(unsigned int *)curr->next->addr ==
					*(unsigned int *)curr->addr + curr->data_size) {
					curr = curr->next;
					mem_in_list += curr->data_size;
				} else {
					break;
				}
			}

			if (mem_in_list <= nr_bytes) {
				found_addr = 0;
				break;
			}
			int bytes_per_node = 0;
			while (nr_bytes > 0) {
				if (!temp->data)
					temp->data = calloc(1, temp->data_size);
				if (nr_bytes < temp->data_size)
					bytes_per_node = nr_bytes;
				else
					bytes_per_node = temp->data_size;
				memcpy(temp->data, data_string, bytes_per_node);
				temp = temp->next;
				nr_bytes -= bytes_per_node;
				data_string += bytes_per_node;
			}
			break;
		}
		curr = curr->next;
		temp = temp->next;
	}

	if (found_addr == 0) {
		printf("Segmentation fault (core dumped)\n");
		*destroy_write = 1;
		dump_mem(sf_list, list, malloc_nr, free_nr, fragment_nr, tot_mem);
		destroy(sf_list, list);
		return;
	}
}

void read(segregated_free_lists *sf_list, allocated_list *list,
		  unsigned int addr, int nr_bytes, int malloc_nr,
	      int free_nr, int fragment_nr, int tot_mem, int *destroy_read)
{
	int found_addr = 0;
	int mem_text = 0;

	block *temp = list->head;
	block *curr = list->head;
	while (curr) {
		if (addr == *(unsigned int *)curr->addr && curr->data) {
			found_addr = 1;
			mem_text = curr->data_size;
			while (curr->next) {
				if (*(unsigned int *)curr->next->addr ==
					*(unsigned int *)curr->addr + curr->data_size) {
					if (curr->next->data) {
						curr = curr->next;
						mem_text += curr->data_size;
					} else {
						break;
					}
				} else {
					break;
				}
			}
			if (mem_text < nr_bytes) {
				found_addr = 0;
				break;
			}

			int bytes_per_node = temp->data_size;
			while (nr_bytes > 0) {
				if (nr_bytes < temp->data_size)
					bytes_per_node = nr_bytes;
				else
					bytes_per_node = temp->data_size;
				for (int i = 0; i < bytes_per_node; i++)
					printf("%c", *((char *)((temp->data) + i)));
				temp = temp->next;
				nr_bytes -= bytes_per_node;
			}
			break;
		}
		temp = temp->next;
		curr = curr->next;
	}

	if (found_addr == 0) {
		printf("Segmentation fault (core dumped)\n");
		*destroy_read = 1;
		dump_mem(sf_list, list, malloc_nr, free_nr, fragment_nr, tot_mem);
		destroy(sf_list, list);
		return;
	}

	printf("\n");
}

int main(void)
{
	segregated_free_lists *sf_list;
	allocated_list *list = calloc(1, sizeof(*list));
	if (!list) {
		fprintf(stderr, "Eroare alocare");
		return -1;
	}

	int malloc_nr = 0;
	int free_nr = 0;
	int fragment_nr = 0;
	int tot_mem = 0;
	int destroy_write = 0; // pt write invalid
	int destroy_read = 0; // pt read invalid

	char cmd[max];

	while (1) {
		fgets(cmd, sizeof(cmd), stdin);

		char *c = strtok(cmd, " "); // pointer la nume comanda doar

		if (strcmp(c, "INIT_HEAP") == 0) {
			char *arg = strtok(NULL, " "); // pointer la argumentele comenzii

			int start_addr = strtol(arg, NULL, 16);
			arg = strtok(NULL, " ");
			int nr_of_lists = atoi(arg);
			arg = strtok(NULL, " ");
			int bytes_per_list = atoi(arg);
			arg = strtok(NULL, "\n");
			int type = atoi(arg);

			sf_list = init_heap(start_addr, nr_of_lists, bytes_per_list);
			tot_mem = sf_list->size * sf_list->bytes_per_list;

		} else if (strcmp(c, "MALLOC") == 0) {
			char *arg = strtok(NULL, "\n");
			int nr_bytes = atoi(arg);
			malloc_function(sf_list, nr_bytes, list,
							&fragment_nr, &malloc_nr);

		} else if (strcmp(c, "FREE") == 0) {
			char *arg = strtok(NULL, "\n");
			int addr = strtol(arg, NULL, 16);
			free_block(list, addr, sf_list, &free_nr);

		} else if (strcmp(c, "WRITE") == 0) {
			char *arg = strtok(NULL, " ");
			int addr = strtol(arg, NULL, 16);
			arg = strtok(NULL, "\"");
			char data_string[max];
			strcpy(data_string, arg);
			arg = strtok(NULL, "\n");
			int nr_bytes = atoi(arg);
			write(sf_list, list, addr, data_string, nr_bytes,
			      malloc_nr, free_nr, fragment_nr, tot_mem, &destroy_write);
			if (destroy_write == 1)
				break;

		} else if (strcmp(c, "READ") == 0) {
			char *arg = strtok(NULL, " ");
			int addr = strtol(arg, NULL, 16);
			arg = strtok(NULL, "\n");
			int nr_bytes = atoi(arg);
			read(sf_list, list, addr, nr_bytes, malloc_nr,
			     free_nr, fragment_nr, tot_mem, &destroy_read);
			if (destroy_read == 1)
				break;

		} else if (strcmp(c, "DUMP_MEMORY\n") == 0) {
			dump_mem(sf_list, list, malloc_nr, free_nr, fragment_nr, tot_mem);

		} else if (strcmp(c, "DESTROY_HEAP\n") == 0) {
			destroy(sf_list, list);
			break;
		}
	}
	return 0;
}
