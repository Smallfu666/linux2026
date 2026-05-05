#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>

static int split_original(int range, int c0, int observations) {
    return range * (c0 + 1) / observations;
}

static int split_safe(int range, int c0, int observations) {
    int64_t num = (int64_t) range * (int64_t) (c0 + 1);
    return (int) (num / observations);
}

static void run_case(const char *name, int range, int c0, int observations) {
    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0) {
        setvbuf(stdout, NULL, _IONBF, 0);
        int safe = split_safe(range, c0, observations);
        printf("case=%s safe_result=%d\n", name, safe);
        int original = split_original(range, c0, observations);
        printf("case=%s original_result=%d\n", name, original);
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
    run_case("near_intmax_small_obs", INT_MAX - 4, INT_MAX - 2, 3);
    run_case("near_intmax_mid_obs", INT_MAX - 1, INT_MAX - 1, 17);
    run_case("realistic", 1024, 37, 64);
    run_case("zeroish", 8, 1, 2);
    return 0;
}
