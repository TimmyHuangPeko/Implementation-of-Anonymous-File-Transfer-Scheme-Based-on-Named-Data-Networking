#include "MD5.h"
#include <string.h>

/*md5轉換用到的常量，算法本身規定的*/
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

static void MD5Transform(UINT4 [4], unsigned char [64]);
static void Encode(unsigned char *, UINT4 *, unsigned int);
static void Decode(UINT4 *, unsigned char *, unsigned int);

/*
用於bits填充的緩沖區，為什么要64個字節呢？因為當欲加密的信息的bits數被512除其余數為448時，
需要填充的bits的最大值為512=64*8 。
*/
static unsigned char PADDING[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
接下來的這幾個宏定義是md5算法規定的，就是對信息進行md5加密都要做的運算。
據說有經驗的高手跟蹤程序時根據這幾個特殊的操作就可以斷定是不是用的md5
*/
/* F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
  Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) {  (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac);  (a) = ROTATE_LEFT ((a), (s));  (a) += (b); }
#define GG(a, b, c, d, x, s, ac) {  (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac);  (a) = ROTATE_LEFT ((a), (s));  (a) += (b); }
#define HH(a, b, c, d, x, s, ac) {  (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac);  (a) = ROTATE_LEFT ((a), (s));  (a) += (b); }
#define II(a, b, c, d, x, s, ac) {  (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac);  (a) = ROTATE_LEFT ((a), (s));  (a) += (b); }

/* MD5 initialization. Begins an MD5 operation, writing a new context. */
/*初始化md5的結構*/
void MD5Init (MD5_CTX *context)
{
    /*將當前的有效信息的長度設成0,這個很簡單,還沒有有效信息,長度當然是0了*/
    context->count[0] = context->count[1] = 0;

    /* Load magic initialization constants.*/
    /*初始化鏈接變量，算法要求這樣，這個沒法解釋了*/
    context->state[0] = 0x67452301;
    context->state[1] = 0xefcdab89;
    context->state[2] = 0x98badcfe;
    context->state[3] = 0x10325476;
}

/* MD5 block update operation. Continues an MD5 message-digest
  operation, processing another message block, and updating the
  context. */
/*將與加密的信息傳遞給md5結構，可以多次調用
context：初始化過了的md5結構
input：欲加密的信息，可以任意長
inputLen：指定input的長度
*/
void MD5Update(MD5_CTX *context,unsigned char * input,unsigned int  inputLen)
{
    unsigned int i, index, partLen;

    /* Compute number of bytes mod 64 */
    /*計算已有信息的bits長度的字節數的模64, 64bytes=512bits。
    用於判斷已有信息加上當前傳過來的信息的總長度能不能達到512bits，
    如果能夠達到則對湊夠的512bits進行一次處理*/
    index = (unsigned int)((context->count[0] >> 3) & 0x3F);

    /* Update number of bits *//*更新已有信息的bits長度*/
    if((context->count[0] += ((UINT4)inputLen << 3)) < ((UINT4)inputLen << 3))
        context->count[1]++;
    context->count[1] += ((UINT4)inputLen >> 29);

    /*計算已有的字節數長度還差多少字節可以 湊成64的整倍數*/
    partLen = 64 - index;

    /* Transform as many times as possible.
    */
    /*如果當前輸入的字節數 大於 已有字節數長度補足64字節整倍數所差的字節數*/
    if(inputLen >= partLen) {
        /*用當前輸入的內容把context->buffer的內容補足512bits*/
        R_memcpy((POINTER)&context->buffer[index], (POINTER)input, partLen);
        /*用基本函數對填充滿的512bits（已經保存到context->buffer中） 做一次轉換，轉換結果保存到context->state中*/
        MD5Transform(context->state, context->buffer);

        /*
        對當前輸入的剩余字節做轉換（如果剩余的字節<在輸入的input緩沖區中>大於512bits的話 ），
        轉換結果保存到context->state中
        */
        for(i = partLen; i + 63 < inputLen; i += 64 )/*把i+63<inputlen改為i+64<=inputlen更容易理解*/
            MD5Transform(context->state, &input[i]);

        index = 0;
    } else
        i = 0;

    /* Buffer remaining input */
    /*將輸入緩沖區中的不足填充滿512bits的剩余內容填充到context->buffer中，留待以后再作處理*/
    R_memcpy((POINTER)&context->buffer[index], (POINTER)&input[i], inputLen-i);
}

/* MD5 finalization. Ends an MD5 message-digest operation, writing the
  the message digest and zeroizing the context. */
/*獲取加密 的最終結果
digest：保存最終的加密串
context：你前面初始化並填入了信息的md5結構
*/
void MD5Final (unsigned char digest[16],MD5_CTX *context)
{
    unsigned char bits[8];
    unsigned int index, padLen;

    /* Save number of bits */
    /*將要被轉換的信息(所有的)的bits長度拷貝到bits中*/
    Encode(bits, context->count, 8);

    /* Pad out to 56 mod 64. */
    /* 計算所有的bits長度的字節數的模64, 64bytes=512bits*/
    index = (unsigned int)((context->count[0] >> 3) & 0x3f);
    /*計算需要填充的字節數，padLen的取值范圍在1-64之間*/
    padLen = (index < 56) ? (56 - index) : (120 - index);
    /*這一次函數調用絕對不會再導致MD5Transform的被調用，因為這一次不會填滿512bits*/
    MD5Update(context, PADDING, padLen);

    /* Append length (before padding) */
    /*補上原始信息的bits長度（bits長度固定的用64bits表示），這一次能夠恰巧湊夠512bits，不會多也不會少*/
    MD5Update(context, bits, 8);

    /* Store state in digest */
    /*將最終的結果保存到digest中。ok，終於大功告成了*/
    Encode(digest, context->state, 16);

    /* Zeroize sensitive information. */

    R_memset((POINTER)context, 0, sizeof(*context));
}

/* MD5 basic transformation. Transforms state based on block. */
/*
對512bits信息(即block緩沖區)進行一次處理，每次處理包括四輪
state[4]：md5結構中的state[4]，用於保存對512bits信息加密的中間結果或者最終結果
block[64]：欲加密的512bits信息
*/
static void MD5Transform (UINT4 state[4], unsigned char block[64])
{
    UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

    Decode(x, block, 64);

    /* Round 1 */
    FF(a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
    FF(d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
    FF(c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
    FF(b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
    FF(a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
    FF(d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
    FF(c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
    FF(b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
    FF(a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
    FF(d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
    FF(c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
    FF(b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
    FF(a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
    FF(d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
    FF(c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
    FF(b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

    /* Round 2 */
    GG(a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
    GG(d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
    GG(c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
    GG(b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
    GG(a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
    GG(d, a, b, c, x[10], S22,  0x2441453); /* 22 */
    GG(c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
    GG(b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
    GG(a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
    GG(d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
    GG(c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
    GG(b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
    GG(a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
    GG(d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
    GG(c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
    GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

    /* Round 3 */
    HH(a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
    HH(d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
    HH(c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
    HH(b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
    HH(a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
    HH(d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
    HH(c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
    HH(b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
    HH(a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
    HH(d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
    HH(c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
    HH(b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
    HH(a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
    HH(d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
    HH(c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
    HH(b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

    /* Round 4 */
    II(a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
    II(d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
    II(c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
    II(b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
    II(a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
    II(d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
    II(c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
    II(b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
    II(a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
    II(d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
    II(c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
    II(b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
    II(a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
    II(d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
    II(c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
    II(b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    /* Zeroize sensitive information. */
    R_memset((POINTER)x, 0, sizeof(x));
}

/* Encodes input (UINT4) into output (unsigned char). Assumes len is
  a multiple of 4. */
/*將4字節的整數copy到字符形式的緩沖區中
output：用於輸出的字符緩沖區
input：欲轉換的四字節的整數形式的數組
len：output緩沖區的長度，要求是4的整數倍
*/
static void Encode(unsigned char *output, UINT4 *input,unsigned int  len)
{
    unsigned int i, j;

    for(i = 0, j = 0; j < len; i++, j += 4) {
        output[j] = (unsigned char)(input[i] & 0xff);
        output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
        output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
        output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
    }
}

/* Decodes input (unsigned char) into output (UINT4). Assumes len is
  a multiple of 4. */
/*與上面的函數正好相反，這一個把字符形式的緩沖區中的數據copy到4字節的整數中（即以整數形式保存）
output：保存轉換出的整數
input：欲轉換的字符緩沖區
len：輸入的字符緩沖區的長度，要求是4的整數倍
*/
static void Decode(UINT4 *output, unsigned char *input,unsigned int  len)
{
    unsigned int i, j;

    for(i = 0, j = 0; j < len; i++, j += 4)
        output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) |
                    (((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
}
