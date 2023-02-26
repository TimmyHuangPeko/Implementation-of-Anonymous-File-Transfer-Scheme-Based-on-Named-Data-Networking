#ifndef _MD5_H_
#define _MD5_H_

#define R_memset(x, y, z) memset(x, y, z)
#define R_memcpy(x, y, z) memcpy(x, y, z)
#define R_memcmp(x, y, z) memcmp(x, y, z)

typedef unsigned long UINT4;
typedef unsigned char *POINTER;

/* MD5 context. */
typedef struct {
  /* state (ABCD) */
  /*四個32bits數，用於存放最終計算得到的消息摘要。當消息長度〉512bits時，也用於存放每個512bits的中間結果*/
  UINT4 state[4];

  /* number of bits, modulo 2^64 (lsb first) */
  /*存儲原始信息的bits數長度,不包括填充的bits，最長為 2^64 bits，因為2^64是一個64位數的最大值*/
  UINT4 count[2];

  /* input buffer */
  /*存放輸入的信息的緩沖區，512bits*/
  unsigned char buffer[64];
} MD5_CTX;

void MD5Init(MD5_CTX *);
void MD5Update(MD5_CTX *, unsigned char *, unsigned int);
void MD5Final(unsigned char [16], MD5_CTX *);

#endif /* _MD5_H_ */
