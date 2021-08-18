#include "main.h"

/**
 * TODO: Processing an executable file
 */

/**
 * The modifiable environment variable for this shell program
 */
static char **ENVP;
/**
 * The number of environment variable in this shell program
 */
static int ENVP_COUNT;
/**
 * The name of the current instance of this shell program
 */
static char *EXEC_NAME;
/**
 * The process ID of the current instance of this shell program
 */
static char *SHELL_PID;
/**
 * The exit code of the last executed process in this shell program
 */
static int NODE_EXIT_CODE;

/**
 * main - Entry point to the simple shell program
 * @ac: The number of arguments passed
 * @av: The arguments passed
 *
 * Return: 0 if successful
 */
int main(int ac, char *av[], char *envp[])
{
	int a = 0, cmd_lines_count = 1;
	char interactive, continuation = FALSE, **file_lines = NULL;
	char *cmd_line = NULL, *tmp = NULL;
	cmd_t *cmds = NULL, *cur = NULL;

	init_shell(ac, av, envp, &cmd_lines_count, &file_lines, &interactive);
	while (a < cmd_lines_count)
	{
		if (interactive && !continuation)
			print_prompt();
		if (interactive && continuation)
			write(STDOUT_FILENO, "> ", 2);
		if (cmd_line == NULL)
			cmd_line = file_lines == NULL ? get_cmd_line() : *(file_lines + a);
		else
			cmd_line = file_lines == NULL ? str_cat(cmd_line, get_cmd_line(), TRUE)
				: *(file_lines + a);
		if (str_len(cmd_line) > 0)
		{
			add_to_history(cmd_line);
			cmds = parse_cmd_line(cmd_line);
			continuation = list_tail(cmds)->command == NULL ? TRUE : FALSE;
			if (continuation)
			{
				free_list(cmds);
				continue;
			}
			cur = cmds;
			while (cur != NULL)
			{
				dissolve_cmd_parts(cur);
				if ((file_lines != NULL) && (cur->next != NULL))
				{
					write(STDOUT_FILENO, "There should be only one command per line\n", 42);
					break;
				}
				/* print_node(cur); */
				/* printf(":: %s, %s\n", cur->command, cmd_line); */
				if (is_built_in_cmd(cur) == TRUE)
				{
					NODE_EXIT_CODE = exec_built_in_cmd(cur);
				}
				else if (is_normal_program(cur, &tmp))
				{
					printf("program: %s\n", tmp);
					NODE_EXIT_CODE = exec_program(cur, tmp);
					free(tmp);
				}
				else
				{
					tmp = long_to_str(get_line_num());
					write(STDERR_FILENO, EXEC_NAME, str_len(EXEC_NAME));
					write(STDERR_FILENO, ": ", 2);
					write(STDERR_FILENO, tmp, str_len(tmp));
					write(STDERR_FILENO, ": ", 2);
					write(STDERR_FILENO, cur->command, str_len(cur->command));
					write(STDERR_FILENO, ": not found\n", 12);
					free(tmp);
					NODE_EXIT_CODE = EC_COMMOAND_NOT_FOUND;
				}
				/* print_node(cur); */
				cur = get_next_command(cur, NODE_EXIT_CODE);
			}
			free_list(cmds);
			free(cmd_line);
			cmd_line = NULL;
		}
		a += (!interactive ? 1 : 0);
	}
	return (NODE_EXIT_CODE);
}

/* TODO: Remove when program is stable */
void print_node(cmd_t *node)
{
	int i;

	printf("cmd: %s, built-in: %d, ac: %d\n", node->command,
					is_built_in_cmd(node), node->args_count);
	for (i = 0; i < node->args_count; i++)
	{
		printf("    arg[%d]: %s \n", i, *(node->args + i));
	}
}

/**
 * init_shell - Entry point to the simple shell program
 * @ac: The number of arguments passed
 * @av: The arguments passed
 * @cmd_lines_count: The number of lines of commands strings in a passed file
 * @file_lines: The lines of command strings in a passed file
 * @interactive: The pointer to the interactive variable
 */
void init_shell(int ac, char *av[], char *envp[],
	int *cmd_lines_count, char ***file_lines, char *interactive)
{
	int fd;

	if (ac > 2)
	{
		write(STDERR_FILENO, "Usage: ", 7);
		write(STDERR_FILENO, av[0], str_len(av[0]));
		write(STDERR_FILENO, "[file]", 6);
		write(STDERR_FILENO, "\n", 1);
		exit(EC_INVALID_ARGS);
	}
	if (ac == 2)
	{
		/* TODO: Load first arg as a file */
		fd = open(av[1], O_RDONLY);
		*file_lines = read_all_lines(fd, cmd_lines_count);
		close(fd);
		exit(127);
	}
	ENVP = envp;
	for (ENVP_COUNT = 0; ENVP[ENVP_COUNT] != NULL; ENVP_COUNT++)
		;
	EXEC_NAME = av[0];
	SHELL_PID = long_to_str(getpid());
	*interactive = (!isatty(STDIN_FILENO) || (ac == 2) ? FALSE : TRUE);
	signal(SIGINT, handle_signal);
	/* signal(SIG_SHELL_ERROR, handle_signal); */
	add_env_var("SHELL", av[0]);
	NODE_EXIT_CODE = 0;
	manage_aliases(MO_INIT);
	manage_history(MO_INIT);
}

/**
 * handle_signal - Handles a signal received by the program
 * @sig_num: The signal's code
 */
void handle_signal(int sig_num)
{
	if (sig_num == SIGINT)
	{
		NODE_EXIT_CODE = EC_CONTROL_C_TERMINATION;
		fflush(stdin);
	}
}

/**
 * get_shell_prop - Retrieved the adddress of a shell prop based on an id
 * @prop_id: The id of the prop to retrieve
 *
 * Return: The pointer to the prop, otherwise NULL
 */
void *get_shell_prop(char prop_id)
{
	switch (prop_id)
	{
	case ENVP_ID:
		return (&ENVP);
	case ENVP_COUNT_ID:
		return (&ENVP_COUNT);
	case EXEC_NAME_ID:
		return (&EXEC_NAME);
	case SHELL_PID_ID:
		return (&SHELL_PID);
	case NODE_EXIT_CODE_ID:
		return (&NODE_EXIT_CODE);
	default:
		break;
	}
	return (NULL);
}

/**
 * clean_up_shell - Frees resources used by this shell instance
 */
void clean_up_shell(void)
{
	/* save_history(); */
	manage_aliases(MO_FREE);
	manage_history(MO_FREE);
}
