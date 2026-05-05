#include <stdio.h>

#define C8(v) ((v) < 0 ? 0 : (v) > 255 ? 255 : (v))

static void run_case(int start) {
    int x = start;
    int result = C8(x++);
    int final_x = x;
    int eval_count = final_x - start;
    printf("initial_x=%d result=%d final_x=%d eval_count=%d interpretation=macro_re_evaluated_operand\n",
           start, result, final_x, eval_count);
}

int main(void) {
    run_case(-1);
    run_case(0);
    run_case(128);
    run_case(255);
    run_case(256);
    return 0;
}
