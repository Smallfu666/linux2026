#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define LIST_POISON1 ((void *)0x100100UL)
#define LIST_POISON2 ((void *)0x200200UL)

#define container_of(ptr, type, member) \
	((type *)((char *)(ptr) - offsetof(type, member)))

struct list_head {
	struct list_head *next;
	struct list_head *prev;
};

struct hlist_node {
	struct hlist_node *next;
	struct hlist_node **pprev;
};

struct hlist_head {
	struct hlist_node *first;
};

typedef struct probe_node {
	const char *name;
	struct probe_node *next;
	struct probe_node **pprev;
} probe_node_t;

struct list_item {
	int value;
	struct list_head link;
};

static void die(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

static void ensure_dir(const char *path)
{
	if (mkdir(path, 0755) == 0 || errno == EEXIST) {
		return;
	}
	die(path);
}

static void init_list_head(struct list_head *head)
{
	head->next = head;
	head->prev = head;
}

static void __list_add(struct list_head *node, struct list_head *prev, struct list_head *next)
{
	next->prev = node;
	node->next = next;
	node->prev = prev;
	prev->next = node;
}

static void list_add_tail(struct list_head *node, struct list_head *head)
{
	__list_add(node, head->prev, head);
}

static void __list_del(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

static void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = (struct list_head *)LIST_POISON1;
	entry->prev = (struct list_head *)LIST_POISON2;
}

static void init_hlist_head(struct hlist_head *head)
{
	head->first = NULL;
}

static void init_hlist_node(struct hlist_node *node)
{
	node->next = NULL;
	node->pprev = NULL;
}

static void hlist_add_head(struct hlist_node *node, struct hlist_head *head)
{
	struct hlist_node *first = head->first;

	node->next = first;
	if (first) {
		first->pprev = &node->next;
	}
	head->first = node;
	node->pprev = &head->first;
}

static void hlist_del(struct hlist_node *node)
{
	if (!node->pprev) {
		return;
	}
	if (node->next) {
		node->next->pprev = node->pprev;
	}
	*node->pprev = node->next;
	node->next = (struct hlist_node *)LIST_POISON1;
	node->pprev = (struct hlist_node **)LIST_POISON2;
}

static const char *ptr_name_probe(const probe_node_t *node)
{
	return node ? node->name : "NULL";
}

static const char *list_next_name(const struct list_head *head, const struct list_head *node, char *buf, size_t cap)
{
	if (node == head) {
		return "HEAD";
	}
	snprintf(buf, cap, "%d", container_of(node, struct list_item, link)->value);
	return buf;
}

static void write_probe_dot_step(FILE *fp, int step, const char *label, const char *pp_label, probe_node_t **pp)
{
	probe_node_t *current = *pp;
	const char *current_name = ptr_name_probe(current);
	const char *next_name = current ? ptr_name_probe(current->next) : "NULL";
	const char *pprev_name = current && current->pprev ? (current->pprev == pp ? pp_label : "other") : "NULL";

	fprintf(fp, "  step%d [shape=record, label=\"{<f0> %s|pp=%s|*pp=%s|next=%s|pprev=%s}\"];\n",
		step, label, pp_label, current_name, next_name, pprev_name);
}

static void write_probe_trace(const char *path, const char *title, probe_node_t **head, const char *target)
{
	FILE *fp = fopen(path, "w");
	if (!fp) {
		die(path);
	}

	fprintf(fp, "digraph \"%s\" {\n", title);
	fprintf(fp, "  rankdir=LR;\n");
	fprintf(fp, "  node [fontname=\"monospace\", shape=record];\n");

	probe_node_t **pp = head;
	char pp_label[64];
	snprintf(pp_label, sizeof(pp_label), "&head");
	int step = 0;

	while (*pp && strcmp((*pp)->name, target) != 0) {
		char step_label[64];
		snprintf(step_label, sizeof(step_label), "seek %s", (*pp)->name);
		write_probe_dot_step(fp, step++, step_label, pp_label, pp);
		snprintf(pp_label, sizeof(pp_label), "&%s.next", (*pp)->name);
		pp = &(*pp)->next;
	}

	if (!*pp) {
		write_probe_dot_step(fp, step++, "miss", pp_label, pp);
		fprintf(fp, "}\n");
		fclose(fp);
		die("target not found");
	}

	write_probe_dot_step(fp, step++, "before delete", pp_label, pp);

	probe_node_t *victim = *pp;
	probe_node_t *next = victim->next;
	*pp = next;
	if (next) {
		next->pprev = pp;
	}
	victim->next = (probe_node_t *)LIST_POISON1;
	victim->pprev = (probe_node_t **)LIST_POISON2;

	write_probe_dot_step(fp, step++, "after delete", pp_label, pp);
	fprintf(fp, "}\n");
	fclose(fp);
}

static void write_iteration_artifact(const char *path)
{
	FILE *fp = fopen(path, "w");
	if (!fp) {
		die(path);
	}

	struct list_head head;
	struct list_item items[4];
	int values[] = { 1, 2, 3, 4 };
	size_t i;

	init_list_head(&head);
	for (i = 0; i < 4; ++i) {
		items[i].value = values[i];
		list_add_tail(&items[i].link, &head);
	}

	fprintf(fp, "SAFE ITERATION\n");
	fprintf(fp, "cached next before delete; this avoids touching poisoned pointers\n");
	for (struct list_head *pos = head.next, *n = pos->next; pos != &head; pos = n, n = pos->next) {
		struct list_item *item = container_of(pos, struct list_item, link);
		char next_buf[16];
		fprintf(fp, "visit %d\n", item->value);
		if ((item->value % 2) == 0) {
			fprintf(fp, "delete %d; next cached as %s\n", item->value,
				list_next_name(&head, n, next_buf, sizeof(next_buf)));
			list_del(pos);
		}
	}

	fprintf(fp, "\nUNSAFE ITERATION\n");
	init_list_head(&head);
	for (i = 0; i < 4; ++i) {
		items[i].value = values[i];
		init_list_head(&items[i].link);
		list_add_tail(&items[i].link, &head);
	}

	for (struct list_head *pos = head.next; pos != &head; pos = pos->next) {
		struct list_item *item = container_of(pos, struct list_item, link);
		fprintf(fp, "visit %d\n", item->value);
		if ((item->value % 2) == 0) {
			list_del(pos);
			fprintf(fp, "delete %d; pos->next is now %p and pos->prev is now %p\n",
				item->value, (void *)pos->next, (void *)pos->prev);
			fprintf(fp, "unsafe loop would advance with pos = pos->next and step onto poison\n");
			break;
		}
	}

	fclose(fp);
}

static void write_summary_artifact(const char *path)
{
	FILE *fp = fopen(path, "w");
	if (!fp) {
		die(path);
	}

	fputs("bc_list_hlist summary\n", fp);
	fputs("- list_del poisons links after unlinking\n", fp);
	fputs("- hlist-style deletion uses the address of the pointer that points at the node\n", fp);
	fputs("- safe iteration caches next before delete; unsafe iteration reads a poisoned next pointer\n", fp);
	fclose(fp);
}

static void run_probe_delete_suite(const char *artifacts_dir)
{
	probe_node_t a = { .name = "A", .next = NULL, .pprev = NULL };
	probe_node_t b = { .name = "B", .next = NULL, .pprev = NULL };
	probe_node_t c = { .name = "C", .next = NULL, .pprev = NULL };
	probe_node_t *head = &a;

	a.next = &b;
	a.pprev = &head;
	b.next = &c;
	b.pprev = &a.next;
	c.next = NULL;
	c.pprev = &b.next;

	char path[PATH_MAX];

	snprintf(path, sizeof(path), "%s/delete_head.dot", artifacts_dir);
	head = &a;
	a.next = &b;
	b.next = &c;
	a.pprev = &head;
	b.pprev = &a.next;
	c.pprev = &b.next;
	write_probe_trace(path, "delete_head", &head, "A");
	if (head != &b || head->next != &c) {
		die("head delete");
	}

	snprintf(path, sizeof(path), "%s/delete_middle.dot", artifacts_dir);
	head = &a;
	a.next = &b;
	b.next = &c;
	a.pprev = &head;
	b.pprev = &a.next;
	c.pprev = &b.next;
	write_probe_trace(path, "delete_middle", &head, "B");
	if (head != &a || head->next != &c) {
		die("middle delete");
	}

	snprintf(path, sizeof(path), "%s/delete_tail.dot", artifacts_dir);
	head = &a;
	a.next = &b;
	b.next = &c;
	a.pprev = &head;
	b.pprev = &a.next;
	c.pprev = &b.next;
	write_probe_trace(path, "delete_tail", &head, "C");
	if (head != &a || head->next != &b || b.next != NULL) {
		die("tail delete");
	}
}

static void run_hlist_smoke(void)
{
	struct hlist_head head;
	struct hlist_node a, b;

	init_hlist_head(&head);
	init_hlist_node(&a);
	init_hlist_node(&b);
	hlist_add_head(&b, &head);
	hlist_add_head(&a, &head);
	if (head.first != &a || a.next != &b || b.pprev != &a.next) {
		die("hlist add");
	}
	hlist_del(&a);
	if (head.first != &b || b.pprev != &head.first) {
		die("hlist delete");
	}
}

int main(int argc, char **argv)
{
	const char *artifacts_dir = argc > 1 ? argv[1] : "artifacts";
	char path[PATH_MAX];

	if (snprintf(path, sizeof(path), "%s", artifacts_dir) >= (int)sizeof(path)) {
		die("artifact path");
	}
	ensure_dir(path);

	run_hlist_smoke();
	run_probe_delete_suite(artifacts_dir);

	snprintf(path, sizeof(path), "%s/iteration.txt", artifacts_dir);
	write_iteration_artifact(path);

	snprintf(path, sizeof(path), "%s/summary.txt", artifacts_dir);
	write_summary_artifact(path);

	return EXIT_SUCCESS;
}
