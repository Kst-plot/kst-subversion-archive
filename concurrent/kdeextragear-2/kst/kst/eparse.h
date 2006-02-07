typedef union {
        char *data;
        double number;
	void *n; /* tree node */
       } YYSTYPE;
#define	T_NUMBER	257
#define	T_IDENTIFIER	258
#define	T_DATA	259
#define	T_ADD	260
#define	T_SUBTRACT	261
#define	T_MULTIPLY	262
#define	T_DIVIDE	263
#define	U_SUBTRACT	264


extern YYSTYPE yylval;
