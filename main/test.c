#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <sys/stat.h>
#include <assert.h>

#if defined(__APPLE__)
#include <sys/time.h>
#endif

static uint64_t
_elapsed() {
#if !defined(__APPLE__)
    struct timespec ti;
    clock_gettime(CLOCK_MONOTONIC, &ti);
    return (uint64_t)ti.tv_sec * 1000 + (uint64_t)ti.tv_nsec / 1000000;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
#endif
}

struct node {
    int value;
};

struct heap {
    int cap;
    int sz;
    struct node p[1];
};

void dump(struct heap *h) {
    fprintf(stderr, "[heap]");
    int i;
    for (i=0;i<h->sz;++i) {
        fprintf(stderr, "%d,", h->p[i].value);
    }
    fprintf(stderr, "[.]\n");
}

void push(struct heap *h, struct node *n) {
    int pos = h->sz;
    while (pos>0) {
        int up = (pos-1)/2;
        if (n->value < h->p[up].value)
            h->p[pos] = h->p[up];
        else break;
        pos=up;
    }
    h->p[pos] = *n;
    h->sz++;     
}

int pop(struct heap *h, struct node* n) {
    if (h->sz <= 0) {
        return 1;
    }
    *n = h->p[0];
    int last = --h->sz;
    if (last > 0) {
        int down = h->p[last--].value;
        int i = 0;
        int child = 1;
        while (child <= last) {
            if (child < last)
                if (h->p[child].value > h->p[child+1].value)
                    child++;
            if (down > h->p[child].value)
                h->p[i] = h->p[child];
            else break;
            i = child;
            child = i*2+1;
        }
        h->p[i] = h->p[last+1];
    }
    return 0;
}

int cmpint(const void *a, const void *b) {
    int i1 = *(int*)a;
    int i2 = *(int*)b;
    return i1-i2;
}

void test(int times) {
    int cap = times;
    struct heap *h = malloc(sizeof(struct heap) + sizeof(struct node) * (cap-1));
    memset(h, 0, sizeof(struct heap) + sizeof(struct node)*(cap-1)); 
    h->cap = cap;
    h->sz = 0;

    struct node n;

    int v[cap];
    int i;
    for (i=0; i<cap; ++i) {
        int value = rand()%100;
        v[i] = value;
        n.value = value;
        push(h, &n);
        //dump(h);
    }
    qsort(v, cap, sizeof(int), cmpint);

    fprintf(stderr, "[v]");
    for (i=0;i<cap;++i) {
        fprintf(stderr, "%d,", v[i]);
    }
    fprintf(stderr, "\n");
    struct node n2;
    for (i=0; i<cap; ++i) {
        assert(!pop(h, &n2));
        //dump(h);
        assert(n2.value == v[i]);
    } 
    assert(pop(h,&n2));
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
 
int test_rsa(int times) {
    //FILE* fp;
    //RSA* rsa1;
/* 
    // check usage
    if (argc != 2) {
        fprintf(stderr, "%s <RSA private key param file>\n", argv[0]);
        exit(-1);
    }
 
    // open the RSA private key PEM file 
    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "Unable to open %s for RSA priv params\n", argv[1]);
        return NULL;
    }
    if ((rsa1 = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL)) == NULL) {
        fprintf(stderr, "Unable to read private key parameters\n");
        return NULL;
    }                                                         
    fclose(fp);
 
    // print 
    printf("Content of Private key PEM file\n");
    RSA_print_fp(stdout, rsa1, 0);
    printf("\n");    
*/ 
/* 
    RSA* rsa2;
 
    // open the RSA public key PEM file
    fp = fopen(argv[2], "r");
    if (fp == NULL) {
        fprintf(stderr, "Unable to open %s for RSA pub params\n", argv[1]);
        return NULL;
    }
    if ((rsa2 = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL)) == NULL) {
        fprintf(stderr, "Unable to read public key parameters\n");
        return NULL;
    }                                                         
    fclose(fp);
 
    printf("Content of Public key PEM file\n");
    RSA_print_fp(stdout, rsa2, 0);
    printf("\n");    
    */
    return 0;
}

typedef unsigned char BYTE;

RSA* read_rsa(const char *file, int public) {
    //const char *file = "/Users/wale/.ssh/test_rsa.pub";
    //const char *file = "./rsakey0.pub";
    BIO *out = BIO_new_file(file, "rb");
    assert(out);
    
    RSA* rsa;
    if (public) {
        rsa = PEM_read_bio_RSA_PUBKEY(out, NULL, NULL, NULL);
    } else {
        rsa = PEM_read_bio_RSAPrivateKey(out, NULL, NULL, NULL);
    }
    //RSA* rsa = PEM_read_bio_RSAPublicKey(out, NULL, NULL, NULL);

    ERR_print_errors_fp(stdout); 
    printf("%p, Content of Public key PEM file\n", rsa);
    RSA_print_fp(stdout, rsa, 0);
    printf("\n");    

    BIO_free(out);
    return rsa;
}
int test_rsa2(int times) {
    //RSA* rsa = RSA_new();
    //BIGNUM *bne = BN_new();
    //BN_set_word(bne, RSA_F4);
    //RSA_generate_key_ex(rsa, 1024 ,bne,NULL);

    RSA* rsa_pri = read_rsa("./rsakey0.pem", 0);
    RSA* rsa_pub = read_rsa("./rsakey0.pub", 1);

    char sz[]="abcdefg";
    int len = RSA_size(rsa_pri);
    BYTE* p = malloc(len);
    memset(p,0,len);
    printf("encrypt:%s, %d\n",sz, len);

    int outlen = RSA_size(rsa_pub)-11;
    char *out = malloc(outlen);
    memset(out, 0, outlen);
    
    int r = RSA_private_encrypt(sizeof(sz), (unsigned char*)sz, p, rsa_pri, RSA_PKCS1_PADDING);
    assert(r!=-1);
    r = RSA_public_decrypt(len,p, (unsigned char*)out, rsa_pub, RSA_PKCS1_PADDING);
    assert(r!=-1);
    //RSA_public_encrypt(sizeof(sz), (unsigned char *)sz, p, rsa, RSA_PKCS1_PADDING);
    //RSA_private_decrypt(len ,p , (unsigned char *)out, rsa, RSA_PKCS1_PADDING);
    printf("decrypt:%s, %d\n",out, outlen);
    free(p);
    free(out);
    RSA_free(rsa_pri);
    RSA_free(rsa_pub);
    return 0;
}

void test2(int time) {
    uint64_t n = 0;
    int i;
    for (i=0; i<1000000000; ++i) {
        n += i;
    }
}

int 
main(int argc, char* argv[]) {
    srand(time(NULL));
    int times = 1;
    if (argc > 1)
        times = strtol(argv[1], NULL, 10);

    uint64_t t1 = _elapsed();
    //test_rsa2(times);
    test2(times);
    uint64_t t2 = _elapsed();
    printf("main use time %d, run times=%d\n", (int)(t2-t1), times);
    return 0;
}
