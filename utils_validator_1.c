#include "main.h"
#include <elf.h>

/**
 * str_is_num - checks if the string is a number
 * @str: the string to check
 *
 * Return: true or false
 */
char str_is_num(char *str)
{
	char *string = str;
	int firstc = 1;

	while (*string != '\0')
	{
		if (firstc)
		{
			if (*string == '-' || *string == '+')
				string++;
			firstc = 0;
			continue;
		}
		if (*string >= '0' && *string <= '9')
			string++;
		else
			return (FALSE);
	}
	return (TRUE);
}

/**
 * is_binary_file - checks if a file is a binary file
 * @fn: file file name
 * Return: True or False
 */
char is_binary_file(char *fn)
{
	unsigned char buf[5];
	int n = 4, fd = open(fn, O_RDONLY);
	const char *s1 = (const char *) buf, *s2 = ELFMAG;

	if (fd == -1)
	{
		perror("couldn't open the file in is_binary_file\n");
		return (-1);
	}
	if (read(fd, buf, n) == -1)
	{
		perror("couldn't read the file in is_binary_file\n");
		close(fd);
		return (-1);
	}
	close(fd);
	/*compare buf with ELFMAG*/
	for ( ; n && *s1 && *s2; --n, ++s1, ++s2)
	{
		if (*s1 != *s2)
			return (FALSE);
	}

	return (TRUE);
}

/**
 * is_variable - Checks if the given string is a variable
 * @str: The string to check
 *
 * Return: TRUE if it is a variable, otherwise FALSE
 */
char is_variable(char *str)
{
	int i = 0;

	while (*(str + i) != '\0')
	{
		if (is_digit(*(str + i)) && i == 0)
			break;
		else if ((is_digit(*(str + i)) && (i > 0))
			|| (is_letter(*(str + i)))
			|| (*(str + i) == '_'))
			i++;
	}
	return (*(str + i) == '\0' && i > 0 ? TRUE : FALSE);
}
