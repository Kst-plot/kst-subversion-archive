#ifndef BISON_EXPARSE_H
# define BISON_EXPARSE_H

#ifndef YYSTYPE
typedef union {
        char *data;
        double number;
	void *n; /* tree node */
       } yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	T_NUMBER	257
# define	T_IDENTIFIER	258
# define	T_DATA	259
# define	T_OR	260
# define	T_AND	261
# define	T_EQ	262
# define	T_NE	263
# define	T_LT	264
# define	T_LE	265
# define	T_GT	266
# define	T_GE	267
# define	T_ADD	268
# define	T_SUBTRACT	269
# define	T_MULTIPLY	270
# define	T_DIVIDE	271
# define	T_NOT	272
# define	U_SUBTRACT	273


extern YYSTYPE exlval;

#endif /* not BISON_EXPARSE_H */
