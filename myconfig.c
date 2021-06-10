#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mychar.h"
#include "mydebug.h"

#define OUTPUT_FILE "Configure"

#define cursor_save() printf("\033[s");
#define cursor_recover() printf("\033[u");

struct config_list {
	char **list;
	int column;
	int column_max;
	struct config_list *next;
	struct config_list *prev;
};

static char* table[][6] = {
	{"ARCH","x86","arcm","arm"},
	{"start","0x0000","0x8080"},
	{"end","0xffff","0x8780"},
	{"","save","cancel"},
};

struct myconfig {
	struct config_list list_head;
	struct config_list *this;
	char *key;
	char *value;
	int raw;
	int column;
	int raw_max;
	int tmp_save;
};
static struct myconfig myconfig;

#define for_each_config_list(head, node) \
	for (node = (head)->next;node != head;node = node->next)

static int config_list_init(struct config_list *head)
{
	head->next = head;
	head->prev = head;
	return 0;
}
static int config_list_add(struct config_list *head, struct config_list *new)
{
	new->prev = head->prev;
	new->next = head;
	head->prev->next = new;
	head->prev = new;
	return 0;
}

static void show_process(char *progress, int time, int show_cursor)
{
	int end = 0;
	int count = 0;
	while (!end) {
		printf("\033[?25l\r%s:%d%% \033[K", progress, count);
		fflush(stdout);
		usleep(time * 1000);
		count ++;
		if (count > 100) {
			end = 1;
		}
	}
	if (show_cursor) {
		printf("\033[?25h\n");
	}else {
		printf("\n");
	}
}

static void table_loading(struct myconfig *cf)
{
	char **list = *table;
	struct config_list *clist;
	int num_list = sizeof(table) / 6 / sizeof(char *);
	show_process("loading", 3, 0);
	config_list_init(&cf->list_head);

	cf->raw = num_list;
	cf->column = 1;
	cf->raw_max = num_list;
	while (num_list --) {
		clist = (struct config_list *)malloc(sizeof(struct config_list));
		if (!clist) {
			perror("\n");
			exit(-1);
		}
		clist->list = list;
		config_list_add(&cf->list_head, clist);
		list += 6;
	}
	cursor_save();
}

static void table_config(struct myconfig *cf)
{
	char buf[40] = {0};
	char key[16] = {0};
	char value[16] = {0};
	char *equal_sign = NULL;
	char **list = NULL;
	struct config_list *node = NULL;
	int column = 0;

	if (access(OUTPUT_FILE, F_OK)) {
		goto err;
	}
	FILE *fp = popen("cat "OUTPUT_FILE, "r");
	if (!fp) {
		perror("\n");
		exit(-1);
	}

	while (fscanf(fp, "%s",buf) != EOF) {
		equal_sign = strchr(buf, '=');
		if (!equal_sign) {
			continue;
		}
		memcpy(key, buf, (equal_sign - buf));
		memcpy(value, (equal_sign + 1), strlen(equal_sign + 1));
		if ((strlen(key) < 1) || (strlen(value) < 1)) {
			continue;
		}

		for_each_config_list(&cf->list_head, node) {
			if (!strncmp(key, *node->list, strlen(key))) {
				column = 0;
				list = (node->list + 1);
				while (*list) {
					column ++;
					if (!strncmp(value, *list, strlen(value))) {
						node->column = column;
						break;
					}
					list ++;
				}
				break;
			}
		}
		memset(buf, 0, sizeof(buf));
		memset(key, 0, sizeof(key));
		memset(value, 0, sizeof(value));
	}

	pclose(fp);
err:
	return;
}

void table_show(struct myconfig *cf)
{
	int i;
	char **list = NULL;
	struct config_list *node = NULL;
	int raw = 0;
	int column = 0;
	cursor_recover();
	printf("/********************************************\n");
	for_each_config_list(&cf->list_head, node){
		raw ++;
		list = node->list;
		if (strlen(*list) > 1) {
			printf("%10s :\t",*list);
		}else {
			printf("%10s  \t",*list);
		}
		while (*(++ list)) {
			if ((++ column) == node->column) {
				printf("\033[31m"); //red
			}
			if (column == cf->column && raw == cf->raw) {
				printf("\033[43m"); //yellow
				cf->this = node;
			}
			printf("%-s",*list);
			printf("\033[0m");
			for (i = 0; i < (10 - strlen(*list)); i ++) {
				printf(" ");
			}
		}
		printf("\n");
		node->column_max = column;
		column = 0;
	}
	printf("********************************************/\n");
}

/**
 * @brief 
 *
 * @return 0:up 1:down 2:left 3:right 10:\n
 */
static int get_direction_key(void)
{
	int direction = 0;
	char key;
try:
	key = getch();
	if (key == 10) {
		return 10;
	}else if (key != 27) {
		goto try;
	}
	key = getch();
	if (key != 91) {
		goto try;
	}
	key = getch();
	switch (key) {
	case 65: //up
		direction = 0;
		break;
	case 66: //down
		direction = 1;
		break;
	case 68: //left
		direction = 2;
		break;
	case 67: //right
		direction = 3;
		break;
	default:
		goto try;
	}
	return direction;
}

static int table_save(struct myconfig *cf)
{
	int ret;
	int fd;
	char **list;
	struct config_list *node;
	fd = open(OUTPUT_FILE, O_RDWR | O_CREAT | O_TRUNC,0664);
	ret = fd;
	serr("open");

	for_each_config_list(&cf->list_head, node) {
		list = node->list;
		if (strlen(*list) > 1 && node->column) {
			ret = write(fd, *list, strlen(*list));
			serr("write");
			ret = write(fd, "=", 1);
			serr("write");
			ret = write(fd, *(list + node->column), strlen(*(list + node->column)));
			serr("write");
			ret = write(fd, "\n", 1);
			serr("write");
		}
	}

	close(fd);
	return 0;
}

static int table_cursor(struct myconfig *cf)
{
	int end = 0;
	int direction = 0;
	do{
		table_show(&myconfig);
		direction = get_direction_key();
		switch (direction) {
		case 0: //up
			cf->raw --;
			cf->column = 1;
			break;
		case 1: //down
			cf->raw ++;
			cf->column = 1;
			break;
		case 2: //left
			cf->column --;
			break;
		case 3: //right
			cf->column ++;
			break;
		case 10:
			if (cf->raw == cf->raw_max) {
				if (cf->column == 1) {
					table_save(cf);
					show_process("saving", 3, 1);
				}else {
					show_process("exiting", 3, 1);
				}
				end = 1;
			}else{
				cf->this->column = cf->column;
			}
			break;
		}
		if (cf->raw > cf->raw_max) {
			cf->raw = 1;
		}
		if (cf->raw < 1) {
			cf->raw = cf->raw_max;
		}
		if (cf->column < 1) {
			cf->column = 1;
		}
		if (cf->column > cf->this->column_max) {
			cf->column = 1;
		}
	}while(!end);
	return 0;
}

int main(int argc, char **argv)
{
	table_loading(&myconfig);
	table_config(&myconfig);
	table_cursor(&myconfig);
	int ret = -1;
	err("sjfiej");
	return 0;
}
