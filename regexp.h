enum
{
	OANY = 0,
	OBOL,
	OCLASS,
	OEOL,
	OJMP,
	ONOTNL,
	ORUNE,
	OSAVE,
	OSPLIT,
	OUNSAVE,
};

typedef struct Resub Resub;
typedef struct Reinst Reinst;
typedef struct Reprog Reprog;
typedef struct Rethread Rethread;

struct Resub
{
	union
	{
		char *sp;
		wchar_t *rsp;
	};
	union
	{
		char *ep;
		wchar_t *rep;
	};
};
struct Reprog
{
	Reinst *startinst;
	Rethread *threads;
	char *regstr;
	int len;
	int nthr;
};

Reprog*	regcomp(char*);
Reprog*	regcomplit(char*);
Reprog*	regcompnl(char*);
void	regerror(char*);
int	regexec(Reprog*, char*, Resub*, int);
void	regsub(char*, char*, int, Resub*, int);
int	rregexec(Reprog*, wchar_t*, Resub*, int);
void	rregsub(wchar_t*, wchar_t*, int, Resub*, int);
