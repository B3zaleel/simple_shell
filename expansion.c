#include "main.h"

/**
 * read_variable - Reads a variable from the given string
 * @str: The string to read from
 * @pos: The position in the string to start from
 *
 * Return: The variable, otherwise NULL
 */
char *read_variable(char *str, int pos)
{
	int i = pos, j, len;
	char *var = NULL;

	while (*(str + i) != '\0')
	{
		if (((*(str + i) == '$') && (i == pos))
			|| ((*(str + i) == '?') && (i == pos)))
		{
			i++;
			break;
		}
		else if ((is_digit(*(str + i)) && (i > pos))
			|| (is_letter(*(str + i)))
			|| (*(str + i) == '_'))
			i++;
		else
			break;
	}
	len = i - pos;
	var = len > 0 ? malloc(sizeof(char) * (len + 1)) : NULL;
	if (var != NULL)
	{
		for (j = 0, i = pos; j < len; j++, i++)
			*(var + j) = *(str + i);
		*(var + j) = '\0';
	}
	return (var);
}

/**
 * dissolve_tokens - Performs a context-aware dissolution of tokens
 * @str: The source string
 * @can_free: Specifies if the given string can be freed
 *
 * Return: The dissolved string, otherwise NULL
 */
char *dissolve_tokens(char *str, char can_free)
{
	size_t size = 0, i = 0, j = 0;
	char incr = 0x40, *res = NULL, quote = FALSE, quote_o = FALSE;

	for (i = 0; *(str + i) != '\0';)
	{
		if (j >= size)
		{
			res = _realloc(res, sizeof(char) * size, sizeof(char) * (size + incr));
			size += incr;
		}
		if (((*(str + i) == '~') && (i == 0)) && ((*(str + 1) == '/') || (*(str + 1) == ':')))
		{
			expand_tilde(str, &i, &res, &j, &size);
		}
		else if (is_quote(*(str + i)))
		{
			if (quote == FALSE)
				quote = *(str + i), quote_o = TRUE, i++;
			else if ((*(str + i) == quote))
				quote = FALSE, quote_o = FALSE, i++;
			else
				*(res + j) = *(str + i), j++, i++;
		}
		else if (*(str + i) == '$')
		{
			if (((quote == '"') && (quote_o == TRUE)) || (quote_o == FALSE))
				expand_variable(str, &i, &res, &j, &size);
			else
				*(res + j) = '$', j++, i++;
		}
		else
		{
			*(res + j) = *(str + i), j++, i++;
		}
	}
	res = _realloc(res, sizeof(char) * j, sizeof(char) * (j + 1));
	if (res != NULL)
		*(res + j) = '\0';
	if (can_free && (str != NULL))
		free(str);
	return (res);
}

void expand_tilde(char *str, size_t *i, char **res, size_t *j, size_t *size)
{
	char *tmp;
	int tmp_len = 0, k = 0;

	(void)str;
	tmp = str_copy(get_env_var("HOME"));
	tmp_len = str_len(tmp);
	if (tmp != NULL)
	{
		if (((*j) + tmp_len) > *size)
		{
			/* allocate space for extra data */
			*res = _realloc(*res, sizeof(char) * (*size),
				sizeof(char) * (*size + ((*j) + tmp_len - (*size))));
			(*size) += ((*j) + tmp_len - (*size));
		}
		for (k = 0; k < tmp_len; k++)
		{
			*(*res + (*j)) = *(tmp + k);
			(*j)++;
		}
		(*i) += 2;
		free(tmp);
		tmp = NULL;
	}
}

void expand_variable(char *str, size_t *i, char **res, size_t *j, size_t *size)
{
	char *var = NULL, *val = NULL;
	int var_len = 0, k, val_len = 0;

	var = read_variable(str, *i + 1);
	if (str_eql("?", var))
		val = long_to_str(*((int *)get_shell_prop(NODE_EXIT_CODE_ID)));
	else if (str_eql("$", var))
		val = long_to_str(*((int *)get_shell_prop(SHELL_PID_ID)));
	else if (var != NULL)
		val = str_copy(get_env_var(var));
	if (val == NULL)
	{
		*(*res + (*j)) = '$';
		(*j)++;
		(*i)++;
	}
	else
	{
		printf("(d_t): %s\n", val);
		/* insert variable */
		var_len = str_len(var) + 1;
		val_len = str_len(val);
		if ((*j + val_len) > *size)
		{
			/* allocate space for extra data */
			*res = _realloc(*res, sizeof(char) * *size,
				sizeof(char) * (*size + (*j + val_len - *size)));
		}
		for (k = 0; k < val_len; k++)
		{
			*(*res + (*j)) = *(val + k);
			(*j)++;
		}
		(*i) += var_len;
		free(val);
		val = NULL;
	}
	if (var != NULL)
		free(var);
}

/**
 * dissolve_cmd_parts - Dissolves the parts of the command node
 * @node: The command node
 */
void dissolve_cmd_parts(cmd_t *node)
{
	int i;

	node->command = dissolve_tokens(node->command, TRUE);
	for (i = 0; i < node->args_count; i++)
	{
		*(node->args + i) = dissolve_tokens(*(node->args + i), TRUE);
	}
}