#include "main.h"

/**
 * get_next_command - Gets the next command that can be executed by the shell
 * @cur: The current command node
 * @exit_code: The exit code of the last command
 *
 * Return: A pointer to the next executable command node
 */
cmd_t *get_next_command(cmd_t *cur, int exit_code)
{
	cmd_t *node = NULL;

	if (((cur->next_cond == OP_OR) && (exit_code != 0))
		|| ((cur->next_cond == OP_AND) && (exit_code == 0)))
	{
		return (cur->next);
	}
	else
	{
		node = cur;
		while (node != NULL)
		{
			if (node->next_cond == OP_SEP)
				return (node->next);
			node = node->next;
		}
		return (node);
	}
}

/**
 * handle_ctrl_d - Handles Control+D key press
 * @len: The number of characters on the command line
 */
void handle_ctrl_d(int len)
{
	char *buf[] = {"0"};

	if (len == 0)
		sc_exit(1, buf);
}

/**
 * check_args - Checks the arguments for a file argument
 * @ac: The number of arguments the program received
 * @av: The arguments the program received
 *
 * Return: TRUE if the file exists and is readable, otherwise FALSE
 */
char check_args(int ac, char *av[])
{
	if (ac > 2)
	{
		write(STDERR_FILENO, "Usage: ", 7);
		write(STDERR_FILENO, av[0], str_len(av[0]));
		write(STDERR_FILENO, " [file]", 7);
		write(STDERR_FILENO, "\n", 1);
		exit(EC_INVALID_ARGS);
	}
	else if (ac == 2)
	{
		if (access(av[1], R_OK) >= 0)
		{
			return (TRUE);
		}
		else
		{
			write(STDERR_FILENO, av[0], str_len(av[0]));
			write(STDERR_FILENO, ": ", 2);
			write(STDERR_FILENO, av[1], str_len(av[1]));
			write(STDERR_FILENO, ": No such file\n", 15);
			exit(EC_INVALID_ARGS);
		}
	}
	return (FALSE);
}
