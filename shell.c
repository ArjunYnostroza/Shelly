#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>


//Declarations for shell commands:
int shelly_cd(char **args);
int shelly_help(char **args);
int shelly_exit(char **args);
int shelly_hello(char **args);

//List of shell commands
char *builtin_str[] = {
    "cd",
    "help",
    "exit",
    "hello"
};

int (*builtin_func[]) (char **) = {
    &shelly_cd,
    &shelly_help,
    &shelly_exit,
    &shelly_hello
};

int shelly_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

char *command_generator(const char *text, int state) {
    static int list_index, len;
    char *name;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    while ((name = builtin_str[list_index])) {
        list_index++;

        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }

    return NULL;
}

char **shelly_command_completion(const char *text, int start, int end) {
    char **matches = NULL;

    if (start == 0) {
        matches = rl_completion_matches(text, command_generator);
    }

    return matches;
}

//#####Command functions#####

//cd
//change directory
//args[0] is cd.  args[1] is directory
//will always returns 1, to continue executing
int shelly_cd(char **args)
{
    if (args[1] == NULL) {
        //fprintf(stderr, "shelly: expected argument to \"cd\"\n");
        char *home = getenv("HOME");
        if (home) {
            if (chdir(home) != 0) {
                perror("shelly");
            }
        } else {
            fprintf(stderr, "shelly: HOME environment variable not set\n");
        }
    } else {
        if (chdir(args[1]) != 0) {
            perror("shelly");
        }
    }
    return 1;
}

//help
//prints startup/help information
int shelly_help(char **args)
{
    printf("This is a basic shell program, with basic functionality.\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("Autocomplete is also built in!\n");
    printf("Partially type in command/arguement and hit the tab key.\n");
    printf("These are the current commands built in:\n");
    for (int i = 0; i < shelly_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }
    printf("\n");
    return 1;
}

//exit
int shelly_exit(char **args)
{
    printf("Bye now! See you soon!\n");
    return 0;
}

//hello
int shelly_hello(char **args)
{
    printf("Hi! Hope you enjoy this shell.\n");
    return 1;
}

//#####End command functions#####

//launcher
//launches program and waits for it to close

int shelly_launch(char **args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        //child
        if (execvp(args[0], args) == -1) {
            perror("shelly");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        //forking error
        perror("shelly");
    } else {
        //parent
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

//execute
//executes shell commands or launches program
//return 1 if  shell should continue running or 0 if it should terminate
int shelly_execute(char **args)
{
    int i;

    if (args[0] == NULL) {
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < shelly_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return shelly_launch(args);
}

//reads line from input
char *shelly_read_line(void) {
    char *line = readline("☀> ");
    if (line && *line) {
        add_history(line);
    }
    return line;
}

#define SHELLY_TOK_BUFSIZE 64
#define SHELLY_TOK_DELIM " \t\r\n\a"

//splits line into tokens
char **shelly_split_line(char *line)
{
    int bufsize = SHELLY_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token, **tokens_backup;

    if (!tokens) {
        fprintf(stderr, "shelly: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, SHELLY_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += SHELLY_TOK_BUFSIZE;
            tokens_backup = tokens;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                free(tokens_backup);
                fprintf(stderr, "shelly: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, SHELLY_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

//shell loop
//recieves input and executes it
void shelly_loop(void) {
    char *line;
    char **args;
    int status;

    // Setup autocompletion
    rl_readline_name = "shelly";
    rl_attempted_completion_function = shelly_command_completion;

    do {
        line = shelly_read_line();
        if (!line) {
            printf("\n");
            exit(0); // Exit on Ctrl-D
        }
        args = shelly_split_line(line);
        status = shelly_execute(args);

        free(line);
        free(args);
    } while (status);
}

//main function
int main(int argc, char **argv)
{
    printf(R"EOF(
      _            _  _        
     | |          | || |       
 ___ | |__    ___ | || | _   _ 
/ __|| '_ \  / _ \| || || | | |
\__ \| | | ||  __/| || || |_| |
|___/|_| |_| \___||_||_| \__, |
                          __/ |
                         |___/

)EOF");

    printf("Welcome to Shelly!\n\n");
    shelly_help(0);

    //runs command loop
    shelly_loop();

    //performs any cleanup.
    return EXIT_SUCCESS;
}
