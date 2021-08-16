#include "main.h"

cmd_t *parse_cmd_line(char *line)
{
	int i = 0, c;
	cmd_t *head = NULL, *node = NULL;
	char prev_token = TKN_BEG, *word, *error = NULL;

	if (line != NULL && *line != '\0')
	{
		for (i = 0; *(line + i) != '#' && *(line + i) != '\0' && error == NULL;)
		{
			if (is_whitespace(*(line + i)))
			{
				prev_token = TKN_SPACE, i++;
			}
			else if (is_operator(*(line + i)))
			{
				read_operator(line, &i, prev_token, &node, &head, &error);
				prev_token = TKN_OP;
			}
			else
			{
				word = read_word(line, &i);
				if (word != NULL)
				{
					if (node == NULL)
					{
						node = new_cmd_node();
						if (node != NULL)
							node->command = word;
						c++;
					}
					else
					{
						node->args = _realloc(node->args, node->args_count * (sizeof(void *)),
																	sizeof(void *) * (node->args_count + 1));
						if (node->args != NULL)
						{
							*(node->args + node->args_count) = word;
							node->args_count++;
						}
					}
				}
				prev_token = TKN_WORD;
			}
		}
		if (node != NULL)
			add_node_to_end(&head, &node);
	}
	return (head);
}

void read_operator(char *line, int *pos, char prev_token,
	cmd_t **head, cmd_t **node, char **error)
{
	int i = *pos, j, op_len = 0;
	char *op_s, op = 0;

	for (j = 0; *(line + i) != '\0' && is_operator(*(line + i)); i++)
	{
		op = op == 0 ? *(line + i) : op;
		if (op != 0 && (*(line + i) != op))
			break;
		j++;
	}
	op_s = j > 0 ? malloc(sizeof(char) * (j + 1)) : NULL;
	i -= j;
	if (op_s != NULL)
	{
		for (j = 0; *(line + i) != '\0' && is_operator(*(line + i)); i++)
			*(op_s + j) = *(line + i), j++;
		*(op_s + j) = '\0', op_len = str_len(op_s);
	}
	if ((op_s != NULL)
		&& ((str_cmp(";", op_s) == 0)
		|| (str_cmp("||", op_s) == 0)
		|| (str_cmp("&&", op_s) == 0)))
	{
		if (node != NULL && *node != NULL)
			(*node)->next_cond = ((*op_s == '|') ? OP_OR
													: ((*op_s == '&') ? OP_AND : OP_SEP));
		if (node != NULL && *node != NULL)
			add_node_to_end(head, node);
	}
	else
	{
		if (((prev_token == TKN_WORD) || (prev_token == TKN_SPACE)))
		{
			*error = str_copy("syntax error near unexpected token '");
		}
		else
		{
			*error = str_copy("syntax error near unexpected token '");
		}
	}
	(void)op_len;
	*pos = i;
}

char *read_word(char *line, int *pos)
{
	int i = *pos, j = *pos, k = 0, quote_o = 0, len = 0;
	char quote = 0, *word;

	while (*(line + i) != '\0')
	{
		if (is_quote(*(line + i)))
		{
			if (quote == 0)
			{
				quote = *(line + i);
				quote_o = 1;
			}
			else if ((*(line + i) == quote))
			{
				quote = 0;
				quote_o = 0;
			}
		}
		else if (quote_o == 0)
		{
			if (is_operator(*(line + i)) || (is_whitespace(*(line + i))) || (*(line + i) == '#'))
				break;
		}
		i++, len++;
	}
	word = malloc(sizeof(char) * (len + 1));
	if (word != NULL)
	{
		for (k = 0; k < len; k++)
		{
			*(word + k) = *(line + j);
			j++;
		}
		*(word + k) = '\0';
	}
	*pos = i;
	return (word);
}


char *read_variable(char *str, int pos)
{
	int i = pos, j, len;
	char *var = NULL;

	printf("[](r_v): %d\n", i);
	while (*(str + i) != '\0')
	{
		if ( ((*(str + i) == '$') && (i == pos))
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
	printf("[](r_v): %d\n", i);
	len = i - pos;
	var = len > 0 ? malloc(sizeof(char) * (len + 1)) : NULL;
	if (var != NULL)
	{
		for (j = 0; j < len; j++)
			*(var + j) = *(str + pos);
		*(var + j) = '\0';
		printf("[](r_v): %s\n", var);
	}
	return (var);
}

char *dissolve_tokens(char *str)
{
	size_t size = 0;
	const char incr = 0x40;
	size_t i = 0, j = 0, k, var_len = 0, val_len = 0;
	char *res = NULL, *var = NULL, *val = NULL, quote = FALSE, quote_o = FALSE;

	for (i = 0; *(str + i) != '\0';)
	{
		if (j >= size)
		{
			res = _realloc(res, sizeof(char) * size, sizeof(char) * (size + incr));
			size += incr;
		}
		if (is_quote(*(str + i)))
		{
			if (quote == FALSE)
			{
				quote = *(str + i);
				quote_o = TRUE;
				i++;
				/* add to deletion */
			}
			else if ((*(str + i) == quote))
			{
				quote = FALSE;
				quote_o = FALSE;
				i++;
				/* add to deletion */
			}
			else
			{
				*(res + j) = *(str + i);
				j++;
				i++;
			}
		}
		else if (*(str + i) == '$')
		{
			if (((quote == '"') && (quote_o == TRUE)) || (quote_o == FALSE))
			{
				var = read_variable(str, i + 1);
				if (str_cmp("?", var) == 0)
					val = long_to_str(*((int *)get_shell_prop(NODE_EXIT_CODE_ID)));
				else if (str_cmp("$", var) == 0)
					val = long_to_str(*((int *)get_shell_prop(SHELL_PID_ID)));
				else if (var != NULL)
					val = str_copy(get_env_var(var));
				if (val == NULL)
				{
					*(res + j) = '$';
					j++;
					i++;
				}
				else
				{
					/* insert variable */
					var_len = str_len(var);
					val_len = str_len(val);
					if ((j + val_len) > size)
					{
						/* allocate space for extra data */
						res = _realloc(res, sizeof(char) * size,
							sizeof(char) * (size + (j + val_len - size)));
					}
					for (k = 0; k < val_len; k++)
					{
						*(res + j) = *(val + k);
						j++;
					}
					i += var_len;
					free(val);
					val = NULL;
				}
				if (var != NULL)
					free(var);
			}
			else
			{
				*(res + j) = '$';
				j++;
				i++;
			}
		}
		else
		{
			*(res + j) = *(str + i);
			j++;
			i++;
		}
	}
	res = _realloc(res, sizeof(char) * j, sizeof(char) * (j + 1));
	*(res + j) = '\0';
	return (res);
}