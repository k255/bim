/*
 * author: Fernando Iazeolla
 * license: GPLv2
 */
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
// #include <editline/readline.h>
// #include <editline/history.h>
#include "main.h"
#ifdef HAVE_LIBREADLINE
#include<readline/readline.h>
#include<readline/history.h>
#endif
#include "repl.h"
#include "ast.h"
#include "env.h"
#include "eval.h"
#include "print.h"
#include "parser.tab.h"

extern FILE *yyin;
extern int errno;
extern int yy_scan_string(char *s);
extern void yylex_destroy(void);

// char* rl_gets(char *s){
// 	char* input = readline(s);
// 	add_history(input);
// 	return input;
// }

void reset_stdin(void){
	freopen("/dev/tty","rb",stdin);
}
char* rl_gets (char *prompt)
{
	static char *line_read = (char *)NULL;
	/* If the buffer has already been allocated,
	 *    return the memory to the free pool. */
	if (line_read)
	{
		free (line_read);
		line_read = (char *)NULL;
	}
	
	/* Get a line from the user. */
	line_read = (char *)readline (prompt);
	
	/* If the line has any text in it,
	 *    save it on the history. */
	if (line_read && *line_read)
		add_history (line_read);
	
	return (line_read);
}
void shell()
{
	char *cmd,prompt[255];
	quit_shell=0;
	int line_no=1;
	//if(check_funny()) funny_shell_disclaimer(); else normal_shell_disclaimer();
	reset_stdin();
	printf("entering shell-interactive mode...\n");
	printf("type 'quit' or 'exit' to quit.\n");
	rl_variable_bind("blink-match-paren","on");
	while(!quit_shell)
	{
		#ifdef HAVE_LIBREADLINE
		sprintf(prompt,".[%d]> ",line_no++);
		cmd=rl_gets(prompt);
		#else
		cmd=(char*)malloc(MAX_CMD);
		fgets(cmd,MAX_CMD,stdin);
		#endif
		execute(cmd);
		#ifndef HAVE_LIBREADLINE
		free(cmd);
		#endif
	}
	
}
void execute(char *s)
{
	struct _object *ast=NULL;
	if(!s) return;
	//printf("pp=%s\n",s);
	yy_scan_string(s);
	int x=yyparse(&ast);
	yylex_destroy();
	log_d("+");
	if(x==0 && ast!=NULL){
		print_obj(run(ast,the_global_environment));
		log_d("ast ");
		if(is_debug_var()){
			printf("\n----------------\n");
			print_ast(ast);
			printf("\n----------------\n");
			print_debug_ast(ast);
			printf("\n----------------\n");
			//print_atom(ast);
			//printf("debug: %d\n",is_debug_var());
		}
		crlf();
		del_cascade(ast);
	}
}
void run_exec(char *buff)
{
	struct _object *ast=NULL;
	if(!buff) return;
	yy_scan_string(buff);
	int x=yyparse(&ast);
	yylex_destroy();
	if(x==0 && ast!=NULL){
		clean_print_obj(run(ast,the_global_environment));
		//run(ast,the_global_environment);
	}
	del_cascade(ast);
	
}
void run_script(char *file)
{
	struct _object *ast=NULL;
	if(!file) return;
	FILE *fp;
	fp=fopen(file,"r");
	if(fp==NULL) {printf("error opening %s file.\n",file); return;}
	yyin=fp;
	int x=yyparse(&ast);
	yylex_destroy();
	if(x==0 && ast!=NULL){
		clean_print_obj(run(ast,the_global_environment));
		//run(ast,the_global_environment);
	}
	del_cascade(ast);
	fclose(fp);
}
