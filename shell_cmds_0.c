#include "main.h"

/**
 * sc_alias - Creates an alias for a command string, prints all the \
 * defined aliases or the value of a single alias
 * @ac: The number of arguments passed
 * @av: The arguments passed
 *
 * Return: The function's exit code
 */
int sc_alias(int ac, char *av[])
{
	int i, n;
	alias_t **aliases = (ac == 0 ? get_aliases(&n) : NULL);
	char *name, *value;

	if (ac == 0)
	{
		for (i = 0; i < n; i++)
		{
			write(STDOUT_FILENO, (*(aliases + i))->name, str_len((*(aliases + i))->name));
			write(STDOUT_FILENO, "='", 2);
			write(STDOUT_FILENO, (*(aliases + i))->value, str_len((*(aliases + i))->value));
			write(STDOUT_FILENO, "'\n", 2);
		}
	}
	else
	{
		for (i = 0; i < ac; i++)
		{
			if (is_variable(av[i]))
			{
				value = get_alias_value(av[i]);
				if (value != NULL)
				{
					write(STDOUT_FILENO, av[i], str_len(av[i]));
					write(STDOUT_FILENO, "='", 2);
					write(STDOUT_FILENO, value == NULL ? "" : value, str_len(value));
					write(STDOUT_FILENO, "'\n", 2);
				}
				else
				{
					write(STDERR_FILENO, "alias: ", 7);
					write(STDERR_FILENO, av[i], str_len(av[i]));
					write(STDERR_FILENO, " not found\n", 11);
					return (1);
				}
			}
			else if (is_name_value_pair(av[i], &name, &value))
			{
				add_alias(name, value);
			}
			else
			{
				return (EC_INVALID_ARGS);
			}
		}
	}
	return (EC_SUCCESS);
}

int sc_cd(int ac, char *av[])
{
	int i;
	char *pwd = get_env_var("PWD");
	char *old_pwd = get_env_var("OLDPWD");

	if (ac <= 0)
	{
		set_env_var("OLDPWD", pwd);
		set_env_var("PWD", get_env_var("HOME"));
		chdir(get_env_var("HOME"));
	}
	else
	{
		if (str_cmp("-", av[0]) == 0)
		{
			/* Switch PWD and OLDPWD */
			if (old_pwd != NULL)
			{
				set_env_var("OLDPWD", pwd);
				set_env_var("PWD", old_pwd);
				chdir(old_pwd);
			}
			else
			{
				printf("OLDPWD not set");
				return (EC_INVALID_ARGS);
			}
		}
		else
		{
			i = chdir(av[0]);
			printf(">> %d, %d\n", i, errno);
			if (i >= 0)
			{
				set_env_var("OLDPWD", pwd);
				set_env_var("PWD", av[0]);
			}
			else
			{
				printf("can't cd to ");
				return (EC_INVALID_ARGS);
			}
		}
	}
	return (EC_SUCCESS);
}

int sc_exit(int ac, char *av[])
{
	int status = EC_SUCCESS;

	if (ac > 0)
	{
		if (str_is_num(av[0]))
		{
			status = str_to_int(av[0]);
		}
		else
		{
			/* TODO: Raise error and add an enum for the exit code */
			write(STDERR_FILENO, "exit: ", 6);
			write(STDERR_FILENO, "Illegal number: ", 16);
			write(STDERR_FILENO, av[0], str_len(av[0]));
			return (2);
		}
	}
	save_history();
	exit(status);
	return (status);
}

/**
 * sc_env - Prints the shell's environment variables
 * @ac: The number of arguments
 * @av: The arguments
 *
 * Return: 0 if successful
 */
int sc_env(int ac, char *av[])
{
	char **envp = *((char ***)get_shell_prop(ENVP_ID));
	int n = *((int *)get_shell_prop(ENVP_COUNT_ID));
	int i;

	(void)ac;
	(void)av;
	for (i = 0; i < n; i++)
	{
		write(STDOUT_FILENO, *(envp + i), str_len(*(envp + i)));
		write(STDOUT_FILENO, "\n", 1);
	}
	return (0);
}

/**
 * sc_help - Prints the help page of a built-in command
 * @ac: The number of arguments
 * @av: The arguments
 *
 * Return: 0 if successful
 */
int sc_help(int ac, char *av[])
{
	int i;
	struct cmd_help cmds_help[] = {
		{"alias", help_alias},
		{"cd", help_cd},
		{"exit", help_exit},
		{"help", help_help},
		{"history", help_history}
	};

	if (ac > 0)
	{
		for (i = 0; i < 9; i++)
		{
			if (str_cmp(cmds_help[i].cmd_name, av[0]) == 0)
			{
				cmds_help[i].run();
				return (0);
			}
		}
	}
	else
	{
		help_program();
	}
	return (0);
}
