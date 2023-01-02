#include <stdio.h>
#include <pwd.h>
#include <unistd.h>

#include "../repl/repl.h"

const char *getUserName() {	
	uid_t uid = geteuid();
	struct passwd *pw = getpwuid(uid);
	if (pw) {
		return pw->pw_name;
	}
	return "";
}

int main(int argc, char *argv[]) {
	printf("Hello %s! This is the Monkey programming language!\n", getUserName());
	printf("Feel free to type in commands\n");
	replStart();
}