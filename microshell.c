/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   testmain.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alilin <alilin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/22 15:10:31 by alilin            #+#    #+#             */
/*   Updated: 2021/03/22 15:54:26 by alilin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int error(char *str)
{
	int i = -1;
	while (str[++i])
		write(STDERR_FILENO, &str[i], 1);
	return (1);
}

int fatal(char **ptr)
{
	free(ptr);
	exit(error("error: fatal\n"));
}

int size_cmd(char **cmd, char *str)
{
	if (!cmd)
		return (0);
	int i = -1;
	while (cmd[++i])
		if (!strcmp(cmd[i], str))
			return (i);
	return (i);
}

char **get_cmd(char **av, int *i)
{
	int size = size_cmd(&av[*i], ";");

	if (!size)
		return (NULL);
	char **tmp;
	if (!(tmp = malloc(sizeof(*tmp) * (size + 1))))
		fatal(NULL);
	int j = -1;
	while (++j < size)
		tmp[j] = av[j + *i];
	tmp[j] = NULL;
	*i += size;
	return (tmp);
}

int cmd_len(char **cmd)
{
	if (!cmd)
		return (0);
	int i = -1;
	while (cmd[++i])
	{
	}
	return (i);
}

int builtin_cd(char **cmd)
{
	if (cmd_len(cmd) != 2)
		return (error("error: cd: bad arguments\n"));
	if (chdir(cmd[1]) < 0)
	{
		error("error: cd: cannot change directory to ");
		error(cmd[1]);
		error("\n");
	}
	return (0);
}

char **find_next_pp(char **cmd)
{
	if (!cmd)
		return (NULL);
	int i = -1;
	while (cmd[++i])
		if (!strcmp(cmd[i], "|"))
			return (&cmd[i + 1]);
	return (NULL);
}

int exec_cmd(char **cmd, char **env, char **ptr)
{
	pid_t pid;

	if ((pid = fork()) < 0)
		fatal(ptr);
	if (!pid)
	{
		if (execve(cmd[0], cmd, env) < 0)
		{
			error("error: cannot execute ");
			error(cmd[0]);
			exit(error("\n"));
		}
	}
	waitpid(0, NULL, 0);
	return (0);
}

int exec_son(char **ptr, char **env, char **tmp, int fd_in, int fd_pp[2])
{
	if (dup2(fd_in, STDIN_FILENO) < 0)
		fatal(ptr);
	if (find_next_pp(tmp) && dup2(fd_pp[1], STDOUT_FILENO) < 0)
		fatal(ptr);
	close(fd_in);
	close(fd_pp[0]);
	close(fd_pp[1]);

	tmp[size_cmd(tmp, "|")] = NULL;
	exec_cmd(tmp, env, ptr);
	free(ptr);
	exit(0);
}

int execute(char **cmd, char **env)
{
	if (!find_next_pp(cmd))
		return (exec_cmd(cmd, env, cmd));
	int fd_in;
	int fd_pp[2];
	int nb_wait = 0;
	char **tmp = cmd;
	pid_t pid;

	if ((fd_in = dup(STDIN_FILENO)) < 0)
		return (fatal(cmd));
	while (tmp)
	{
		if (pipe(fd_pp) < 0 || (pid = fork()) < 0)
			fatal(cmd);
		if (!pid)
			exec_son(cmd, env, tmp, fd_in, fd_pp);
		else
		{
			if (dup2(fd_pp[0], fd_in) < 0)
				fatal(cmd);
			close(fd_pp[0]);
			close(fd_pp[1]);
			++nb_wait;
			tmp = find_next_pp(tmp);
		}
	}
	close(fd_in);
	while (nb_wait-- >= 0)
		waitpid(0, NULL, 0);
	return (0);
}

int main(int ac, char **av, char **env)
{
	char **cmd;
	int i = 0;

	cmd = NULL;
	while (++i < ac)
	{
		cmd = get_cmd(av, &i);
		if (cmd && !strcmp(cmd[0], "cd"))
			builtin_cd(cmd);
		else if (cmd)
			execute(cmd, env);
		free(cmd);
		cmd = NULL;
	}
	return (0);
}
