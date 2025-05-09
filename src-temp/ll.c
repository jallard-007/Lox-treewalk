#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Tt Tt;
struct Tt {
    char *this;
    Tt *next;
    Tt *prev;
};

typedef struct TtList TtList;
 struct TtList {
    Tt *head;
    Tt *tail;
};

void insert_after(TtList *l, Tt *n, Tt *s) {
    s->prev = n;
    if (n->next) {
        s->next = n->next;
        s->next->prev = s;
    } else {
        l->tail = s;
    }
    n->next = s;
}

void insert_before(TtList *l, Tt *n, Tt *s) {
    s->next = n;
    if (n->prev) {
        s->prev = n->prev;
        s->prev->next = s;
    } else {
        l->head = s;
    }
    n->prev = s;
}

void insert_end(TtList *l, Tt *s) {
    if (l->tail) {
        insert_after(l, l->tail, s);
    } else {
        l->head = s;
        l->tail = s;
    }
}

void insert_start(TtList *l, Tt *s) {
    if (l->head) {
        insert_before(l, l->head, s);
    } else {
        l->head = s;
        l->tail = s;
    }
}

void l_remove(TtList *l, Tt *s) {
    if (s->next) {
        s->next->prev = s->prev;
    } else {
        l->tail = s->prev;
    }
    if (s->prev) {
        s->prev->next = s->next;
    } else {
        l->head = s->next;
    }
    s->next = NULL;
    s->prev = NULL;
}

Tt* find(TtList *l, Tt *s) {
    Tt *i = l->head;
    while (i) {
        if (strcmp(i->this, s->this) == 0) {
            return i;
        }
        i = i->next;
    }
    return NULL;
}

int main() {
    Tt t = {0};
    char tbuf [20];
    t.this = tbuf;
    strcpy(t.this, "hello world!\n");
    TtList l = {0};
    insert_end(&l, &t);

    Tt v = {0};
    char vbuf [20];
    v.this = vbuf;
    strcpy(v.this, "hello world!\n");
    Tt* f = find(&l, &v);
    if (!f) {
        printf("not found\n");
    } else {
        printf("%s", f->this);
    }

    l_remove(&l, &v);
    f = find(&l, &v);
    if (!f) {
        printf("not found\n");
    } else {
        printf("%s", f->this);
    }
    return 0;
}
