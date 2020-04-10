#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "regexp.h"
#include "regimpl.h"

typedef struct RethreadQ RethreadQ;
struct RethreadQ {
	Rethread *head;
	Rethread **tail;
};

int
regexec(Reprog *p, char *str, Resub *sem, int msize)
{
	RethreadQ lists[2], *clist, *nlist, *tmp;
	Rethread *t, *next, *pool, *avail;
	Reinst *ci;
	wchar_t r;
	char *sp, *ep, endc;
	int i, matchgen, gen, mbmax;

	memset(p->threads, 0, sizeof(Rethread)*p->nthr);
	if(msize > NSUBEXPM)
		msize = NSUBEXPM;
	if(p->startinst->gen != 0) {
		for(ci = p->startinst; ci < p->startinst + p->len; ci++)
			ci->gen = 0;
	}

	clist = lists;
	clist->head = NULL;
	clist->tail = &clist->head;
	nlist = lists + 1;
	nlist->head = NULL;
	nlist->tail = &nlist->head;

	pool = p->threads;
	avail = NULL;
	gen = matchgen = 0;

	sp = str;
	ep = NULL;
	mbmax = MB_CUR_MAX;
	endc = '\0';
	if(sem != NULL && msize > 0) {
		if(sem->sp != NULL)
			sp = sem->sp;
		if(sem->ep != NULL && *sem->ep != '\0') {
			ep = sem->ep;
			endc = *sem->ep;
			*sem->ep = '\0';
		}
	}

	for(r = L'☺'; r != L'\0'; sp += i) {
		i = mbtowc(&r, sp, mbmax);
		if(i <= 0)
			break;
		gen++;
		if(matchgen == 0) {
			if(avail == NULL) {
				assert(pool < p->threads + p->nthr);
				t = pool++;
			} else {
				t = avail;
				avail = avail->next;
			}
			t->i = p->startinst;
			if(msize > 0)
				memset(t->sem, 0, sizeof(Resub)*msize);
			t->next = NULL;
			t->gen = gen;
			*clist->tail = t;
			clist->tail = &t->next;
		}
		t = clist->head;
		if(t == NULL)
			break;
		ci = t->i;
Again:
		if(ci->gen == gen)
			goto Done;
		ci->gen = gen;
		switch(ci->op) {
		case ORUNE:
			if(r != ci->r)
				goto Done;
		case OANY: /* fallthrough */
			next = t->next;
			t->i = ci + 1;
			t->next = NULL;
			*nlist->tail = t;
			nlist->tail = &t->next;
			goto Next;
		case OCLASS:
		Class:
			if(r < ci->r)
				goto Done;
			if(r > ci->r1) {
				ci++;
				goto Class;
			}
			next = t->next;
			t->i = ci->a;
			t->next = NULL;
			*nlist->tail = t;
			nlist->tail = &t->next;
			goto Next;
		case ONOTNL:
			if(r != L'\n') {
				ci++;
				goto Again;
			}
			goto Done;
		case OBOL:
			if(sp == str || sp[-1] == '\n') {
				ci++;
				goto Again;
			}
			goto Done;
		case OEOL:
			if(r == L'\n' || (r == L'\0' && ep == NULL)) {
				ci++;
				goto Again;
			}
			goto Done;
		case OJMP:
			ci = ci->a;
			goto Again;
		case OSPLIT:
			if(avail == NULL) {
				assert(pool < p->threads + p->nthr);
				next = pool++;
			} else {
				next = avail;
				avail = avail->next;
			}
			next->i = ci->b;
			if(msize > 0)
				memcpy(next->sem, t->sem, sizeof(Resub)*msize);
			next->next = t->next;
			next->gen = t->gen;
			t->next = next;
			ci = ci->a;
			goto Again;
		case OSAVE:
			if(ci->sub < msize)
				t->sem[ci->sub].sp = sp;
			ci++;
			goto Again;
		case OUNSAVE:
			if(ci->sub == 0) {
				matchgen = t->gen;
				if(sem != NULL && msize > 0) {
					memcpy(sem, t->sem, sizeof(Resub)*msize);
					sem->ep = sp;
				}
				goto Done;
			}
			if(ci->sub < msize)
				t->sem[ci->sub].ep = sp;
			ci++;
			goto Again;
		Done:
			next = t->next;
			t->next = avail;
			avail = t;
		Next:
			if(next == NULL)
				break;
			if(matchgen && next->gen > matchgen) {
				*clist->tail = avail;
				avail = next;
				break;
			}
			t = next;
			ci = t->i;
			goto Again;
		}
		tmp = clist;
		clist = nlist;
		nlist = tmp;
		nlist->head = NULL;
		nlist->tail = &nlist->head;
	}
	if(ep != NULL)
		*ep = endc;
	return matchgen > 0 ? 1 : 0;
}
