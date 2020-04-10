enum {
	LANY = 0,
	LBOL,
	LCLASS,
	LEND,
	LEOL,
	LLPAR,
	LOR,
	LREP,
	LRPAR,
	LRUNE,

	TANY = 0,
	TBOL,
	TCAT,
	TCLASS,
	TEOL,
	TNOTNL,
	TOR,
	TPLUS,
	TQUES,
	TRUNE,
	TSTAR,
	TSUB,

	NSUBEXPM = 32,
};

typedef struct Parselex Parselex;
typedef struct Renode Renode;

struct Parselex {
	/* Parse */
	Renode *next, *nodes;
	int sub, instrs;
	jmp_buf exitenv;
	/* Lex */
	void (*getnextr)(Parselex*);
	char *rawexp, *orig;
	wchar_t rune;
	int peek, peeklex, done, literal, nc;
	wchar_t cpairs[400+2];
};

struct Renode {
	int op;
	Renode *left;
	wchar_t r;
	union {
		wchar_t r1;
		int sub;
		Renode *right;
	};
	int nclass;
};

struct Rethread {
	Reinst *i;
	Resub sem[NSUBEXPM];
	Rethread *next;
	int gen;
};

struct Reinst {
	char op;
	int gen;
	Reinst *a;
	union {
		wchar_t r;
		int sub;
	};
	union {
		wchar_t r1;
		Reinst *b;
	};
};

/*
static int lex(Parselex*);
static void getnextr(Parselex*);
static void getnextrlit(Parselex*);
static void getclass(Parselex*);
static Renode *e0(Parselex*);
static Renode *e1(Parselex*);
static Renode *e2(Parselex*);
static Renode *e3(Parselex*);
static Renode *buildclass(Parselex*);
static Renode *buildclassn(Parselex*);
static int pcmp(void*, void*);
static Reprog *regcomp1(char*, int, int);
static Reinst *compile(Renode*, Reprog*, int);
static Reinst *compile1(Renode*, Reinst*, int*, int);
static void prtree(Renode*, int, int);
*/
