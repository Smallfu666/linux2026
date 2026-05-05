#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static int parse_pair_buggy(const char *s) {
    int hi = s[0] - '0';
    if (hi > 9) {
        hi -= 'a' - '9' - 1;
    }
    int lo = s[1] - '0';
    if (lo > 9) {
        lo -= 'a' - '9' - 1;
    }
    return (hi << 4) | lo;
}

static void run_case(const char *name, const char *input, size_t len) {
    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0) {
        setvbuf(stdout, NULL, _IONBF, 0);
        char *buf = (char *) malloc(len + 1);
        if (!buf) {
            perror("malloc");
            _exit(1);
        }
        memcpy(buf, input, len);
        buf[len] = '\0';
        int value = parse_pair_buggy(buf);
        printf("case=%s input=%s value=%d\n", name, len ? buf : "\"\"", value);
        free(buf);
        _exit(0);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    printf("case=%s child_exit=%d child_signal=%d\n",
           name,
           WIFEXITED(status) ? WEXITSTATUS(status) : -1,
           WIFSIGNALED(status) ? WTERMSIG(status) : 0);
}

int main(void) {
    run_case("0f", "0f", 2);
    run_case("0F", "0F", 2);
    run_case("AF", "AF", 2);
    run_case("GG", "GG", 2);
    run_case("f", "f", 1);
    run_case("empty", "", 0);
    return 0;
}
