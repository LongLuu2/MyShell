# myshell

`myshell` is a lightweight, custom-built Unix shell written in C. It supports basic command execution, input/output redirection, built-in commands, and piping. The shell also includes functionality for background processes and handles multiple commands in a single input line.

## Features

### Core Features
- **Command Execution**: Execute standard Unix commands by specifying the program name and its arguments.
- **Piping (`|`)**: Chain commands using pipes to redirect the output of one command to another.
- **Input/Output Redirection**:
  - Redirect output using `>` or `1>`.
  - Redirect input using `<`.
  - Redirect error output using `2>`.
  - Redirect both output and error using `&>`.
- **Background Processes (`&`)**: Run commands in the background.
- **Command Separators (`;`)**: Execute multiple commands sequentially within a single input line.

### Built-In Commands
1. **`ls`**: List files in the current or specified directory.
2. **`cd`**: Change the current working directory.
3. **`help`**: Display a list of supported commands.

### Additional Features
- **Custom Input Parsing**: Parses input to handle special characters like `|`, `>`, `<`, `&`, and `;`.
- **Dynamic Memory Management**: Dynamically allocates and resizes memory for command arguments and arrays.
- **Interactive and Non-Interactive Modes**: Works interactively (with a prompt) or non-interactively (e.g., via script input).


