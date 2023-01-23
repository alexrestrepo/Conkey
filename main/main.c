#include <stdio.h>
#include <pwd.h>
#include <unistd.h>

#include "../repl/repl.h"

const char *getUserName(void) {
	uid_t uid = geteuid();
	struct passwd *pw = getpwuid(uid);
	if (pw) {
		return pw->pw_name;
	}
	return "";
}

int main(int argc, char *argv[]) {
    // if argc load file and run that, otherwise repl

	printf("Hello %s! This is the Monkey programming language!\n", getUserName());
	printf("Feel free to type in commands\n");
	replStart();
}

