#include"RSA.h"




void print(LINT x){
	int n = 0;
	
	for(int a = SIZE - 1 ; a >= 0 ; a--){
		if(n == 0 && x.t[a] == 0)
			continue;
		n = 1;
		printf("%02x", x.t[a]);
	}
	if( !n )
		printf("%02x", x.t[0]);
	printf("\n");
}

int GET_Byte(LINT X){
	
	int n;
	
	for(n = SIZE - 1 ; n >= 0 ; n--){
		if( X.t[n] != 0 )
			break;
	}
	
	return n+1;
	
}

LINT trans(long long x){
	long long n = x;
	
	LINT r;
	for(int a = 0 ; a < SIZE ; a++)
		r.t[a] = 0;
	
	int a = 0;
	while( n > 0 ){
		r.t[a] = n % 256;
		n /= 256;
		a++;
	}
	
	//r.size = a - 1;
	
	return r;
}

LINT add(LINT x, LINT y){
	int n = 0;
	LINT r;
	for(int a = 0 ; a < SIZE ; a++)
		r.t[a] = 0;
	
	for(int a = 0 ; a < SIZE - 1 ; a++){
	
		if(n)
			r.t[a] = x.t[a] + y.t[a] + 1;
		else
			r.t[a] = x.t[a] + y.t[a];
		
		if(r.t[a] < x.t[a])
			n = 1;
		else if(n && r.t[a] == x.t[a])
			n = 1;
		else
			n = 0;
			
	}
	
	return r;
}

LINT sub(LINT x, LINT y){
	int n = 0;
	LINT r;
	for(int a = 0 ; a < SIZE ; a++)
		r.t[a] = 0;
	
	for(int a = 0 ; a < SIZE - 1 ; a++){
	
		if(n)
			r.t[a] = x.t[a] - y.t[a] - 1;
		else
			r.t[a] = x.t[a] - y.t[a];
		
		if(r.t[a] > x.t[a])
			n = 1;
		else if(n && r.t[a] == x.t[a])
			n = 1;
		else
			n = 0;
			
	}
	
	return r;
}

LINT right_one(LINT x){
	for(int a = 0 ; a < SIZE - 1 ; a++){
		x.t[a] = x.t[a] >> 1;
		if(x.t[a+1] % 2 == 1)
			x.t[a] += 128;
	}
	//x.size -= 1;
	return x;
}

LINT right_eight(LINT x){
	
	for(int a = 0 ; a < SIZE - 1 ; a++){
		x.t[a] = x.t[a+1];
	}
	x.t[SIZE - 1] = 0;
	//x.size -= 1;
	return x;
}

LINT left_one(LINT x){
	for(int a = SIZE - 1 ; a > 0 ; a--){
		x.t[a] = x.t[a] << 1;
		if(x.t[a-1] >= 128)
			x.t[a] += 1;
	}
	x.t[0] = x.t[0] << 1;
	//x.size -= 1;
	return x;
}


LINT left_eight(LINT x){
	
	for(int a = SIZE - 1 ; a > 0; a--){
		x.t[a] = x.t[a-1];
	}
	x.t[0] = 0;
	//x.size -= 1;
	return x;
}

int equal(LINT x, LINT y){
	
	int i;
	
	for(i = SIZE - 1 ; i >= 0 ; i--)
		if(x.t[i] != y.t[i])
			break;
	
	if(x.t[i] > y.t[i])
		return 1;
	else if(x.t[i] < y.t[i])
		return -1;
	else
		return 0;
		 
}

LINT mul(LINT x, LINT y){
	
	LINT ZERO = trans(0);
	LINT r = trans(0);
	LINT m = x;
	
	while( equal(y, ZERO) == 1 ){
		
		if(y.t[0] % 2 == 1)
			r = add(r, m);
		
		m = left_one(m);
		y = right_one(y);
		
	}
	
	return r;
}

LINT mul2(LINT x, LINT y){
	
	LINT ZERO = trans(0);
	LINT r = trans(0);
	LINT m = x;
	int i = 0;
	
	while( equal(y, ZERO) == 1 ){
		
		int n = 0;
		for(int a = 0 ; a < SIZE - 1 ; a++){
			
			n = x.t[a] * y.t[0] + n / 256 + r.t[a+i];
			r.t[a+i] = n % 256;
			
		}
		i++;
		y = right_eight(y);
		
	}
	
	return r;
}

LINT Pow(LINT x, LINT y){
	
	LINT ZERO = trans(0);
	LINT r = trans(1);
	LINT m = x;
	
	while( equal(y, ZERO) == 1 ){
		
		if(y.t[0] % 2 == 1)
			r = mul2(r, m);
		
		m = mul2(m, m);
		y = right_one(y);
		
		/*printf("============\n");
		print(y);
		printf("============\n");*/
	}
	
	return r;
}

LINT divid(LINT x, LINT y){
	
	int n, i;
	
	if( equal(x, y) < 0 )
		return trans(0);
	
	LINT r;
	
	
	for(int a = 0 ; a < SIZE ; a++)
		r.t[a] = 0;
	
	for(i = SIZE - 1 ; i >= 0 ; i--)
		if(y.t[i] != 0)
			break;
	
	for(n = SIZE - 1 ; n >= 0 ; n--)
		if(x.t[n] != 0)
			break;
	
	
	
	
	LINT Y = y;
	
	for(int a = 0 ; a < n - i ; a++){
		Y = left_eight(Y);
	}
	
	while( equal(x, y) >= 0 ){
		if(equal(x, Y) >= 0){
			x = sub(x, Y);
			r.t[0]++;
		}
		else{
			Y = right_eight(Y);
			r = left_eight(r);
		}
	}
	
	
	return r;
}

LINT mod(LINT x, LINT y){
	
	int n, i;
	for(i = SIZE - 1 ; i >= 0 ; i--)
		if(y.t[i] != 0)
			break;
	
	for(n = SIZE - 1 ; n >= 0 ; n--)
		if(x.t[n] != 0)
			break;
	
	if(n < i)
		return x;
	
	
	LINT Y = y;
	
	for(int a = 0 ; a < n - i ; a++){
		Y = left_eight(Y);
	}
	
	while( equal(x, y) >= 0 ){
		if(equal(x, Y) >= 0)
			x = sub(x, Y);
		else
			Y = right_one(Y);
	}
	
	
	return x;
}

LINT modular(LINT x, LINT y, LINT n){
	LINT ZERO = trans(0);
	LINT r = trans(1);
	LINT m = x;
	
	while( equal(y, ZERO) == 1 ){
		
		if(y.t[0] % 2 == 1){
			r = mul2(r, m);
			r = mod(r, n);
		}
		
		m = mul2(m, m);
		y = right_one(y);
		
		
		m = mod(m, n);
		/*printf("============\n");
		print(y);
		printf("============\n");*/
	}
	
	return mod(r, n);
}

LINT GET_K(int* prime, int size){
	
	LINT R = trans(1);
	
	while( GET_Byte(R) < MIN_K){
		R = mul( R, trans(prime[ rand() % size ]) );
	}
	
	return R;
}

int gcd(int a, int b){
	
	int r = a % b;
	
	if(r == 0)
		return b;
	else
		return gcd(b, r);
}

LINT GCD(LINT a, LINT b){
	LINT r = mod(a, b);
	
	if( equal(r, trans(0)) == 0)
		return b;
	else
		return GCD(b, r);
}

int exgcd(int a, int b){
	int s1 = 1;
	int s2 = 0;
	int t1 = 0;
	int t2 = 1;
	
	while(b > 0){
		int q = a / b;
		
		int r = a - q * b;
		a = b;
		b = r;
		
		int s = s1 - q * s2;
		s1 = s2;
		s2 = s;
		
		int t = t1 - q * s2;
		t1 = t2;
		t2 = t;
	}
	
	return s1;
}

LINT EXGCD(LINT a, LINT b){
	LINT s1 = trans(1);
	LINT s2 = trans(0);
	LINT zero = trans(0);
	LINT n = b;
	LINT s;
	int n1 = 0;
	int n2 = 0;
	int ns = 0;
	while(equal(b, zero) > 0){
		LINT q = divid(a, b);
		
		LINT r = sub(a, mul2(q, b));
		r = mod(a,b);
		a = b;
		b = r;
		
		
		/*print(q);
		print(s1);*/
		
		//printf("\n=======================\n\n");
		
		LINT sm = mul2(q, s2);
		
		/*print(a);
		print(b);
		print(q);*/
		
	
		if( n1 != n2 ){
			s = add(s1, sm);
			ns = n1;
		}
		else{
			if( equal(s1, sm) >= 0 ){
				s = sub( s1, sm);
				ns = n1;
			}
			else{
				s = sub( sm, s1);
				ns = !n1;
			}
		}
		
		
		
		s1 = s2;
		s2 = s;
		
		n1 = n2;
		n2 = ns;
		
		
	}
	
	if(n1){
		s1 = sub(n, s1);
	}
	
	while( equal(s1, n) >= 0 )
		s1 = sub(s1,n);
	
	return s1;
}

RSA Creat_Key(){
	
	RSA result;
	
	char *table = (char*) malloc(MAX_P);
	int *prime = (int*) malloc(sizeof(int)*MAX_P);
	int i = 0;
	
	for(int a = 0 ; a < MAX_P ; a++)
		table[a] = 0;
	
	for(int a = 2 ; a < MAX_P ; a++){
		if(table[a] == 1)
			continue;
		for(int b = a ; b < MAX_P ; b+=a)
			table[b] = 1;
		
		prime[i++] = a;
		//printf("%d ",a);
	}
	//printf("\n");
	
	LINT q = trans(1);
	LINT p = trans(1);
	
	int q_table[P_N], p_table[P_N];
	int q_t[P_N], p_t[P_N];
	
	
	for( int a = 0 ; a < P_N ; a++ ){
		q_table[a] = prime[ rand() % i ] ;
		q_t[a] = 0;
		p_t[a] = 0;
	}
	
	for( int a = 0 ; a < P_N ; a++ ){
		p_table[a] = prime[ rand() % i ];
		
		char f = 0;
		
		for( int b = 0 ; b < P_N ; b++ ){
			if( p_table[a] == q_table[b] ){
				f = 1;
				break;
			}
		}
		
		if(f)
			a--;
	}
	
	
	while( GET_Byte(q) < MAX_N ){
		int I = rand() % P_N;
		q = mul( q, trans(q_table[I]));
		q_t[I]++; 
	}
	
	while( GET_Byte(p) < MAX_N ){
		int I = rand() % P_N;
		p = mul( p, trans(p_table[I]));
		p_t[I]++; 
	}
	
	LINT N = mul(q, p);
	
	
	
	LINT r = trans(1);
	LINT e;
	int e_table[P_N];
	do{
		
		
		for(int a = 0 ; a < P_N ; a++){
			
			if(q_t[a] != 0){
				LINT x = trans(q_table[a]);
				x = sub( Pow(x, trans(q_t[a])) , Pow(x, trans(q_t[a] - 1)) );
				r = mul(r, x);
			}
			if(p_t[a] != 0){
				LINT y = trans(p_table[a]);
				y = sub( Pow(y, trans(p_t[a])) , Pow(y, trans(p_t[a] - 1)) );
				r = mul(r, y);
			}
			
			/*printf("R : ");
			print(r);*/
		}
			
		for(int a = 0 ; a < P_N ; a++){
			e_table[a] = prime[ rand() % i ];
			
			int f = 0;
			
			for( int b = 0 ; b < P_N ; b++ ){
				if( gcd( e_table[a], p_table[b] - 1 ) != 1 ){
					f = 1;
					break;
				}
				else if( gcd( e_table[a], q_table[b] - 1 ) != 1 ){
					f = 1;
					break;
				}
			}
			
			if(f)
				a--;
		}
		e = GET_K(e_table, P_N);
	}while( equal(GCD(e,r), trans(1)) != 0 );
	
	LINT d = EXGCD(e, r);
	
	result.e = e;
	result.d = d;
	result.n = N;
	
	return result;
}

LINT RSA_Encrypt(unsigned char* mes, int size, LINT e, LINT n){
	
	LINT text = trans(0);
	for(int a = 0 ; a < size ; a++)
		text.t[a] = mes[a];
		
	/*printf("\nTEXT : ");
	print(text);*/
	
	return modular(text, e, n);
}

void RSA_Decrypt(unsigned char* mes, int size, LINT e, LINT d, LINT n){
	
	LINT r = modular(e,d,n);
	
	for(int a = 0 ; a < size ; a++)
		mes[a] = r.t[a];
	
	
}
