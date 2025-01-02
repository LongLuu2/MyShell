#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>

struct cmd {
    char **cmd;
    int infd, outfd, errfd, b_flag;
};
//function for reading line
char *read_line() {
    char *line = NULL;
    ssize_t buf = 0;

    if (getline(&line, &buf, stdin) == -1) {
        if (feof(stdin)) {
            printf("exit\n");
            exit(EXIT_SUCCESS); 
        } else {
            perror("read_line");
            exit(EXIT_FAILURE); 
        }
    }
    return line;
}

//function for parsing the line
char **parse_line(char *line) {
    int pos = 0;
    int buff = 10;
    char **result = malloc(buff * sizeof(char*));

    char *token = strtok(line, " \t\r\n\a");

    while(token != NULL) {
        if (token[strlen(token) - 1] == ';') {
            token[strlen(token) - 1] = '\0';
            result[pos++] = strdup(token);
            result[pos++] = strdup(";");
        } else {
        result[pos++] = strdup(token);
        
        }
        token = strtok(NULL," \t\r\n\a" );
        if (pos >= buff) {
            buff += buff;
            result = realloc(result, buff *sizeof(char*));
        }
    }
    result[pos] = NULL;
    return result;

}

//built in functions
void ls(char **args, int in, int out, int err);
void cd(char **args, int in, int out, int err);
void help(char **args, int in, int out, int err);

char *builtin_fun_names[] = {
    "ls",
    "cd",
    "help",
    NULL

};
//helper functions

int getCmdCount() {
     int cmd_count = 0;
    //get count of commands
    while (builtin_fun_names[cmd_count] != NULL) {
        cmd_count++;
    }
    return cmd_count;
}

//command functions

//ls command
void ls(char **args, int in, int out, int err) {
    DIR *dp;
    struct dirent *dirp;
    int argc = 0;

    //get count of args
    while (args[argc] != NULL) {
        argc++;
    } 

    //print the files in directory 
    if(argc > 2) {
        write(err, "ls command: ls pathname\n", strlen("ls command: ls pathname\n"));
        return;
    } else if(argc == 2) {// user specified path
        if ((dp = opendir(args[1])) == NULL)
        write(err, "can not open file", strlen("can not open file"));
	    while ((dirp = readdir(dp)) != NULL){
            write(out, dirp->d_name, strlen(dirp->d_name));
            write(out, " ", 1);
        }
    } else { // current dir
        dp = opendir(".");
        while ((dirp = readdir(dp)) != NULL) {
            write(out, dirp->d_name, strlen(dirp->d_name));
            write(out, " ", 1);
        }
    }
    closedir(dp);
    write(out, "\n", 1);
    return;
}
//cd command
void cd(char **args, int in, int out, int err) {
    int argc = 0;

    //get count of args
    while (args[argc] != NULL) {
        argc++;
    }

    if(argc != 2) {
        write(err, "cd command: cd dirname", strlen("cd command: cd dirname"));
    } else {
        if(chdir(args[1]) != 0) {
            write(err, "dir does not exist", strlen("dir does not exist"));
        }
    }
    return;
}

//help command
void help(char **args, int in, int out, int err) {
    write(out, "support for following commands: \n", strlen("support for following commands: \n"));
    for (int i = 0; i < getCmdCount(); i++){
        write(out, builtin_fun_names[i], strlen(builtin_fun_names[i]));
        write(out, " ", 1);
    }
    write(out, "\n", 1);
    return;
}

void(*builtin_fun[]) (char **, int, int, int) = {
    &ls,
    &cd,
    &help
};

//helper function to adjust args after redirect io
void shiftLeft(char **shift_this, int pos) {
    int j;
    for (j = pos; shift_this[j] != NULL && shift_this[j+1] != NULL; j++) {
    shift_this[j] = shift_this[j+2];
}
shift_this[j] = NULL;
}

//change stdInOutErr fds base on user input
void determine_fds(char **input_args, int *infd, int *outfd, int *errfd) {
    int args_count = 0;

    while (input_args[args_count] != NULL) 
        args_count++;

    //finds symbol and update fd
    for(int i = 0; i < args_count; i++) {
        if (strlen(input_args[i]) == 1 && input_args[i][0] == '>'){
            int fd = open(input_args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd == -1){ 
                perror("can not open");
                exit(127);
            }

            //change stdout
            *outfd = fd;
            shiftLeft(input_args, i);
            i += 2;
        } else if (strlen(input_args[i]) == 1 && input_args[i][0] == '<') {
            int fd = open(input_args[i+1], O_RDONLY);
            if (fd == -1){ 
                perror("can not open");
                exit(127);
            } 
            //change stdin
            *infd = fd;
            shiftLeft(input_args, i);
            i += 2;
        } else if (strlen(input_args[i]) == 2 && (strcmp(input_args[i], "1>") == 0)) {
            int fd = open(input_args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd == -1){ 
                perror("can not open");
                exit(127);
            }

            //change stdout
            *outfd = fd;
            shiftLeft(input_args, i);
            i += 2;
        } else if (strlen(input_args[i]) == 2 && (strcmp(input_args[i], "2>") == 0)) {
            int fd = open(input_args[i+1], O_WRONLY);
            if (fd == -1){ 
                perror("can not open");
                exit(127);
            } 

            *errfd = fd;
            shiftLeft(input_args, i);
            i += 2;

        } else if (strlen(input_args[i]) == 2 && (strcmp(input_args[i], "&>") == 0)) {
            int fd = open(input_args[i+1], O_RDWR);
            if (fd == -1){ 
                perror("can not open");
                exit(127);
            } 

            *infd = fd;
            *outfd = fd;
            shiftLeft(input_args, i);
            i += 2;
        }
        
    }
    
}

// checks for built in commands first before starting process
void process(char **args, int infd, int outfd, int errfd, int back_flag);
void run(char **args, int infd, int outfd, int errfd, int back_flag) {
    int cmd_count = getCmdCount();
    
    if (args[0] == NULL) {
        return;
    }

    
    for(int i = 0; i < cmd_count; i++ ) {
        if(strcmp(args[0], builtin_fun_names[i]) == 0) {
            (*builtin_fun[i]) (args, infd, outfd, errfd);
            if (infd != STDIN_FILENO) close(infd);
            if (outfd != STDOUT_FILENO) close(outfd);
            if (errfd != STDERR_FILENO) close(errfd);
            return; 
        }
    } 
    //starts process
    process(args, infd, outfd, errfd, back_flag);
    return;
}

// code for process
void process(char **args, int infd, int outfd, int errfd, int back_flag) {
    pid_t pid;
    pid_t wpid;
    int status;
    pid = fork();

    if (pid == 0) {
        dup2(infd, 0);
        dup2(outfd, 1);
        dup2(errfd, 2);
        if (infd != STDIN_FILENO) close(infd);
        if (outfd != STDOUT_FILENO) close(outfd);
        if (errfd != STDERR_FILENO) close(errfd);
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("could not execute");
            exit(EXIT_FAILURE);
        }
        if (infd != STDIN_FILENO) close(infd);
        if (outfd != STDOUT_FILENO) close(outfd);
    } else if (pid < 0) {
        // Forking failed
        perror("fork failed");
    } else {
        // Parent process
        if (infd != STDIN_FILENO) close(infd);
        if (outfd != STDOUT_FILENO) close(outfd);
        if (errfd != STDERR_FILENO) close(errfd);
        if (!back_flag) {
            do {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            }
    }
    return;
}

int main()
{
    char *line;
    char **args;
    int loop = 1;
  

    do {
        if (isatty(STDIN_FILENO)){
        printf("myshell> ");
        }

        line = read_line();
        args = parse_line(line);
        int args_count = 0;

        while (args[args_count] != NULL){
            //printf("%s \n", args[args_count]);
            args_count++;
        } 

        int cmd_index = 0;
        //array of commands with their i/o and args
        struct cmd *cmd_array = malloc(80 * sizeof(struct cmd));

        //fd for pipes
        int fds[2]; 
        fds[0] = fds[1] = -1; 

        for(int i = 0; i < args_count;) { 
            int cmd_size = 0;
            
            //get cmd up to ; or |
            while (args[i + cmd_size] != NULL && args[i + cmd_size][0] != ';' && 
            args[i + cmd_size][0] != '|' && args[i + cmd_size][0] != '&') {
                cmd_size++;
            }
            //printf("\n");
            char **parsed_cmd = malloc((cmd_size + 1) * sizeof(char *));
            for (int j = 0; j < cmd_size; j++) {
                parsed_cmd[j] = args[i + j];
            }
            parsed_cmd[cmd_size] = NULL;

            int infd = STDIN_FILENO;
            int outfd = STDOUT_FILENO;
            int errfd = STDERR_FILENO;

            determine_fds(parsed_cmd, &infd, &outfd, &errfd);
            cmd_array[cmd_index].cmd = parsed_cmd;
            cmd_array[cmd_index].infd = infd;
            cmd_array[cmd_index].outfd = outfd;
            cmd_array[cmd_index].errfd = errfd;

            //check if there was a pipe in previous process and give current process the read fd, 
            //Which full connects the pipes
            if(fds[0] != -1) {
                cmd_array[cmd_index].infd = fds[0];
                fds[0] = fds[1] = -1;
            }

            //check for pipe symbol, give the output fd to this process
            if(args[i + cmd_size] != NULL && args[i + cmd_size][0] == '|' ) {
                pipe(fds);
                cmd_array[cmd_index].outfd = fds[1];
            }  
            if (args[i + cmd_size] != NULL && args[i + cmd_size][0] == '&') {
                cmd_array[cmd_index].b_flag = 1;
            } else {
                cmd_array[cmd_index].b_flag = 0;
            }



            cmd_index++;
            i += cmd_size + 1;
            //printf("\nthis:%s\n", args[i]);
            //printf("%d", i);
        }
        
        for(int i = 0; i < cmd_index; i++) {
            run(cmd_array[i].cmd, cmd_array[i].infd,cmd_array[i].outfd, cmd_array[i].errfd, cmd_array[i].b_flag );
            //printf("%s, %d, %d, %d\n", cmd_array[i].cmd[0], cmd_array[i].infd,cmd_array[i].outfd, cmd_array[i].errfd, cmd_array[1].b_flag);
            free(cmd_array[i].cmd);
        }
        // printf("count: %d\n", cmd_index);


        for (int i = 0; args[i] != NULL; i++) {
        free(args[i]);
        }
        free(line);
        free(args);
        free(cmd_array);
        close(fds[0]);
        close(fds[1]);

    }while(loop);

    return 0;
}

