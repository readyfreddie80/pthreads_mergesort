#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>
#include <sys/time.h>

static int count = 0;

pthread_mutex_t lock;

typedef struct scalar_ctx_t {
    int M;
    int *up;
    int *down;
    int left;
    int right;
} scalar_ctx_t;

void initialization(int *up, int N, int a, int b) {
    srand(time(NULL));
    for(int i = 0; i < N; i++) {
        up[i] = rand() % (b - a + 1) + a;
    }
}

int cmp(const void *a, const void *b) {
     return *(int*)a - *(int*)b;
 }


void* merge_sort(void *context) {

    scalar_ctx_t *ctx = context;
    int width = ctx->right - ctx->left + 1;
    if (width <= ctx->M) {
          qsort(ctx->up + ctx->left, width, sizeof(int), cmp);
          return ctx->up;
    }

    int middle = (int)((ctx->left + ctx->right) * 0.5);

    // сортировка
    int *l_buff;
    int *r_buff;

    scalar_ctx_t left_ctx = {
        .up = ctx->up,
        .down = ctx->down,
        .left = ctx->left,
        .right = middle,
        .M = ctx->M,
    };

    scalar_ctx_t right_ctx = {
        .up = ctx->up,
        .down = ctx->down,
        .left = middle + 1,
        .right = ctx->right,
        .M = ctx->M,
    };

    int child_created = 0;
    pthread_t child;
    void *returnValue;

    pthread_mutex_lock(&lock);
    if(count - 1 > 0) {
        count--;
        child_created = 1;
    }
    pthread_mutex_unlock(&lock);

    if(child_created == 1) {
         pthread_create(&child, NULL, merge_sort, &left_ctx);
    }
    else
        l_buff = (int*)merge_sort(&left_ctx);

    r_buff = (int*)merge_sort(&right_ctx);

    if(child_created == 1) {
         pthread_join(child, &returnValue);
         pthread_mutex_lock(&lock);
         count++;
         pthread_mutex_unlock(&lock);
         l_buff = (int*)returnValue;  
		
    }


    int *target = l_buff == ctx->up ? ctx->down : ctx->up;


    int l_cur = ctx->left, r_cur = middle + 1;

    for (int i = ctx->left; i <= ctx->right; i++){

        if (l_cur <= middle && r_cur <= ctx->right) {
            if (l_buff[l_cur] < r_buff[r_cur]){
                target[i] = l_buff[l_cur];
                l_cur++;
            } else {
                target[i] = r_buff[r_cur];
                r_cur++;
                }
        } else if (l_cur <= middle) {
            target[i] = l_buff[l_cur];
            l_cur++;
        }
        else {
            target[i] = r_buff[r_cur];
            r_cur++;
        }

    }
 
    return target;
}




int main(int argc, char** argv) {

    int N = atoi(argv[1]);
    int M = atoi(argv[2]);
    int P = atoi(argv[3]);


    int *up = (int*)malloc(sizeof(int) * N);

    initialization(up, N, 0, 200);
    pthread_mutex_init(&lock, NULL);
    count = P;

    FILE *data = fopen("data.txt", "w");

    for (int i = 0; i < N; i++) {
        fprintf(data, "%d ", up[i]);
    }

    struct timeval start, end;
    int *down = (int*)malloc(sizeof(int) * N);

     scalar_ctx_t ctx = {
        .up = up,
        .down = down,
        .left = 0,
        .right = N - 1,
        .M = M,
    };

    assert(gettimeofday(&start, NULL) == 0);
    int *res = (int*)merge_sort(&ctx);
    assert(gettimeofday(&end, NULL) == 0);
    double delta = ((end.tv_sec - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;

    FILE *stats = fopen("stats.txt", "w");
    fprintf(stats, "%.5fs %d %d %d\n", delta, N, M, P);
    fclose(stats);

    fprintf(data, "\n");
    for (int i = 0; i < N; i++) {
        fprintf(data, "%d ", res[i]);
    }
    pthread_mutex_destroy(&lock);
    fclose(data);
    free(up);
    free(down);
    return 0;
}

