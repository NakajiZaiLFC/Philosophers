
# The Norm - Version 4.1

## Contents

- [The Norm - Version 4.1](#the-norm---version-41)
	- [Contents](#contents)
	- [I. Foreword](#i-foreword)
	- [II. Why?](#ii-why)
	- [III. The Norm](#iii-the-norm)
		- [III.1 Naming](#iii1-naming)
		- [III.2 Formatting](#iii2-formatting)
		- [III.3 Functions](#iii3-functions)
		- [III.4 Typedef, struct, enum and union](#iii4-typedef-struct-enum-and-union)
		- [III.5 Headers - a.k.a include files](#iii5-headers---aka-include-files)
		- [III.6 The 42 header - a.k.a start a file with style](#iii6-the-42-header---aka-start-a-file-with-style)
		- [III.7 Macros and Pre-processors](#iii7-macros-and-pre-processors)
		- [III.8 Forbidden stuff!](#iii8-forbidden-stuff)
		- [III.9 Comments](#iii9-comments)
		- [III.10 Files](#iii10-files)
		- [III.11 Makefile](#iii11-makefile)

---

## I. Foreword

The `norminette` is a Python and open source tool to check Norm compliance. Not all rules are checked by it — subjective rules are marked with (*) and can lead to project failure.

GitHub repo: [https://github.com/42School/norminette](https://github.com/42School/norminette)

---

## II. Why?

The Norm is designed to help beginners:

- **Sequencing:** Simplifies code structure. Max 25 lines per function, avoid complex constructs.
- **Look and Feel:** Standardized naming, formatting to ease collaboration.
- **Long-term Vision:** Maintainable and understandable code is easier to debug and extend.
- **References:** All rules have rationale — readability, maintainability, and simplicity.

---

## III. The Norm

### III.1 Naming

- Struct: starts with `s_`
- Typedef: `t_`
- Union: `u_`
- Enum: `e_`
- Global var: `g_`
- Use only lowercase, digits, `_` (snake_case)
- ASCII only (except in literals)
- (*) Names should be explicit, mnemonic, English-readable
- Globals must be `const` or `static`, unless explicitly allowed
- Files must compile to pass Norm

---

### III.2 Formatting

- Max 25 lines per function (excluding braces)
- Max 80 columns per line
- Functions separated by one empty line
- Use **tab characters**, not spaces (ASCII 9)
- Braces on their own lines (except in struct, enum, union declarations)
- No trailing whitespace or tabs
- No multiple consecutive empty lines or spaces
- Declarations at function start
- Pointer asterisks stick to variable name
- One variable per line
- Declaration + initialization not allowed (except static/global/const)
- Empty line between declarations and logic
- One instruction per line
- Split long instructions using natural spaces; operators start new lines
- Space after `,` and `;` unless end of line
- One space around operators
- One space after C keywords (except types and `sizeof`)
- Control structures need braces unless one-liner

Example:
```c
int g_global;

typedef struct s_struct
{
	char *my_string;
	int i;
} t_struct;

int main(void)
{
	int i;
	char c;

	return (i);
}
```

---

### III.3 Functions

- Max 4 named parameters
- Use `void` if no params
- Prototypes must name parameters
- Max 5 variables per function
- Return values in `()` (unless void)
- Single tab between return type and name

Example:
```c
int my_func(int arg1, char arg2, char *arg3)
{
	return (my_val);
}

int func2(void)
{
	return ;
}
```

---

### III.4 Typedef, struct, enum and union

- Space after `struct`, `union`, `enum`
- Indent names per block scope
- Follow indentation rules inside braces
- Space after `typedef`
- Structures must not be declared in `.c` files

---

### III.5 Headers - a.k.a include files

- (*) Allowed: includes, declarations, defines, prototypes, macros
- Includes must be at top
- Never include `.c` files
- Protect with `#ifndef`, e.g., `FT_FOO_H`
- (*) No unused headers
- Justify inclusions with comments

Example:
```c
#ifndef FT_HEADER_H
# define FT_HEADER_H

# include <stdlib.h>
# include <stdio.h>
# define FOO "bar"

int g_variable;
struct s_struct;

#endif
```

---

### III.6 The 42 header - a.k.a start a file with style

- All `.c`/`.h` files must start with 42 header (multi-line comment)
- (*) Must contain creator login/email, creation/update dates

> Note: May need to manually configure header for your info.

---

### III.7 Macros and Pre-processors

- (*) `#define` only for literal/constant values
- (*) No `#define` to bypass Norm or obfuscate code
- (*) Only allowed macros from standard libraries
- No multiline macros
- Macro names must be UPPERCASE
- Indent inside `#if`, `#ifdef`, `#ifndef`
- No preprocessor code outside global scope

---

### III.8 Forbidden stuff!

You cannot use:

- `for`
- `do...while`
- `switch`, `case`
- `goto`
- Ternary operators (`?`)
- VLAs (e.g., `char str[argc]`)
- Implicit typing

Example:
```c
char str[argc]; // VLA — forbidden
i = argc > 5 ? 0 : 1; // Ternary — forbidden
```

---

### III.9 Comments

- No comments **inside** function bodies
- (*) Must be in English and useful
- (*) Cannot justify bad code or functions with vague names like `f1`, `a`, etc.

---

### III.10 Files

- Never include `.c` files
- Max 5 function definitions per `.c` file

---

### III.11 Makefile

Makefiles not checked by norminette but must follow rules:

- Must define: `all`, `clean`, `fclean`, `re`, `$(NAME)`
- `all` is default
- Must not relink unnecessarily
- For multi-binary projects: define rule per binary
- Auto-compile local libraries (e.g., libft)
- List source files explicitly — no `*.c`, `*.o`, etc.

---
