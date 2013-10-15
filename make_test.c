// src3.4-18 �����肵�Ă��錾�ꃂ�f���ɂ��f�[�^�𐶐������C
// src3.4-18 �Ō��̃��f��������ł��邩�V�~�����[�V����

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define PI 3.141592653589793238
#define E  2.718281828459045235


#define M 40000 // ����
#define V 50  // �P��قȂ萔
#define MVT   51  // max(V,T)+1
#define Alpha 0.2 
#define GA    0.02
#define Beta  0.2
#define GB    0.02
#define T     22
#define End   0.1  // P(end | w) �̕��ϒl

// �p�����[�^ a[0],a[1],�c, a[n-1] �̃f�B���N�����z�ɏ]������ (X1,X2,�c,Xn) �𐶐�
// �������������i�̑g�j�́C�����̕ϐ��ŕԂ����icall for reference�j

double gamma_rnd(double a)  /* �K���}���z, a > 0 */
{
	double t, u, x, y;

	if (a > 1) {
		t = sqrt(2 * a - 1);
		do {
			do {
				/* ���̂S�s�� y = tan(PI * drand48()) �Ɠ��l */
				do {
					x = 1 - drand48();  y = 2 * drand48() - 1;
				} while (x * x + y * y > 1);
				y /= x;
				x = t * y + a - 1;
			} while (x <= 0);
			u = (a - 1) * log(x / (a - 1)) - t * y;
		} while (u < -50 || drand48() > (1 + y * y) * exp(u));
	} else {
		t = E / (a + E);
		do {
			if (drand48() < t) {
				x = pow(drand48(), 1 / a);  y = exp(-x);
			} else {
				x = 1 - log(drand48());  y = pow(x, a - 1);
			}
		} while (drand48() >= y);
	}
	return x;
}

void dirichlet(int n, double * a, double * x)
{
  int i;
  double *y;
  double sum=0;
  y = (double *)malloc(n*sizeof(double));
  
  for(i=0 ; i<n ; i++){
    y[i] = gamma_rnd(a[i]);
    sum += y[i];
  }
  for(i=0 ; i<n ; i++)
    x[i] = y[i]/sum;
}


// p(0), p(1), �c, p(n-1) �ɏ]���ă����_���� i (0�� i �� n-1�j��I��
double q[MVT];
int randomSelect(int n, double *p){
  int i;
  double r;
  q[0]=p[0];
  for(i=1 ; i<n ; i++)
    q[i] = q[i-1] + p[i];

  r=drand48();
  for(i=0 ; i<n ; i++)
    if(r<=q[i])
      return i;

  printf("something wrong. n=%d\n",n);
}


double theta[V][V][2];
double phi[V][T];
double zeta[V][V][T];
double pw[V][V+1];  // ID=V �͏I�[��\�킷����L��
double pw1[V];

int numW[V];
int numWZ[V][T];
int numWW0[V][V];
int numWW1[V][V];
int numWWZ[V][V][T];

int main(){
  int i,t,m,w1,w2,w;
  double alpha[2],beta[T];
  double sum,R;

  for(w=0 ; w<V ; w++){
    numW[w]=0;
    for(t=0 ; t<T ; t++)
      numWZ[w][t]=0;
  }

  for(w1=0 ; w1<V ; w1++)
    for(w2=0 ; w2<V ; w2++){
      numWW0[w1][w2]=0;
      numWW1[w1][w2]=0;
      for(t=0 ; t<T ; t++)
	numWWZ[w1][w2][t]=0;
    }


  //  0 --> 000000  0
  //  1 --> 000001  1
  //  2 --> 000010  2
  //  3 --> 000100  4
  //  4 --> 001000  8
  //  5 --> 010000 16
  //  6 --> 100000 32
  //  7 --> 000011  3
  //  8 --> 000101  5
  //  9 --> 001001  9
  // 10 --> 010001 17
  // 11 --> 100001 33
  // 12 --> 000110  6
  // 13 --> 001010 10
  // 14 --> 010010 18
  // 15 --> 100010 34
  // 16 --> 001100 12
  // 17 --> 010100 20
  // 18 --> 100100 36
  // 19 --> 011000 24
  // 20 --> 101000 40
  // 21 --> 110000 48
  
  int type[T];  // id --> value type id
  type[ 0] =  0;
  type[ 1] =  1;
  type[ 2] =  2;
  type[ 3] =  4;
  type[ 4] =  8;
  type[ 5] = 16;
  type[ 6] = 32;
  type[ 7] =  3;
  type[ 8] =  5;
  type[ 9] =  9;
  type[10] = 17;
  type[11] = 33;
  type[12] =  6;
  type[13] = 10;
  type[14] = 18;
  type[15] = 34;
  type[16] = 12;
  type[17] = 20;
  type[18] = 36;
  type[19] = 24;
  type[20] = 40;
  type[21] = 48;

  int type2v[64][7], tmp;
  for(t=0 ; t<64 ; t++) {
    // type t �����l�σx�N�g�� v2 �ɕϊ�
    tmp = t;
    for(i=6 ; i>=1 ; i--){
      type2v[t][i] = tmp % 2;
      tmp = tmp / 2;
    }
    type2v[t][0]=1;
  }
  
  alpha[0]=Alpha*GA;
  alpha[1]=Alpha;

  beta[0] = Beta;
  for(i=1 ; i<T ; i++) beta[i]=Beta*GB;

  FILE *fp;

  // theta[w1][w2][*] �̐ݒ�
  for(w1=0 ; w1<V ; w1++)
    for(w2=0 ; w2<V ; w2++)
      dirichlet(2, alpha, &(theta[w1][w2][0]));

  fp=fopen("model-theta","w");
  for(w1=0 ; w1<V ; w1++)
    for(w2=0 ; w2<V ; w2++)
      if(theta[w1][w2][0]>0.1)
	fprintf(fp,"theta[%d][%d][0]= %f  [1]= %f\n",w1,w2,theta[w1][w2][0],theta[w1][w2][1]);
  fclose(fp);
  
  // phi[w][*] �̐ݒ�
  for(w=0 ; w<V ; w++)
    dirichlet(T, beta, &(phi[w][0]));

  fp=fopen("model-phi","w");
  for(w=0 ; w<V ; w++){
    for(i=1 ; i<T ; i++)
      if(phi[w][i]>0.1)
	fprintf(fp,"phi[%d][%d]= %f\n",w,i,phi[w][i]);
    fprintf(fp,"\n");
  }
  fclose(fp);

  // zeta[w1][w2][*] �̐ݒ�
  for(w1=0 ; w1<V ; w1++)
    for(w2=0 ; w2<V ; w2++)
      dirichlet(T, beta, &(zeta[w1][w2][0]));

  fp=fopen("model-zeta","w");
  for(w1=0 ; w1<V ; w1++)
    for(w2=0 ; w2<V ; w2++){
      for(i=0 ; i<T ; i++)
	if(zeta[w1][w2][i]>0.1)
	  fprintf(fp,"zeta[%d][%d][%d]= %f\n",w1,w2,i,zeta[w1][w2][i]);
      fprintf(fp,"\n");
    }
  fclose(fp);

  // �P�� bigram pw[w1][w2] = P(w2|w1) �̐ݒ�
  for(w1=0 ; w1<V ; w1++){
    sum=0;
    for(w2=0 ; w2<V ; w2++){
      pw[w1][w2] = drand48();
      sum += pw[w1][w2];
    }
    R = 2*drand48()*End;
    pw[w1][V] = sum*R/(1-R);
    sum += pw[w1][V];
    for(w2=0 ; w2<=V ; w2++)
      pw[w1][w2] /= sum;
  }

  // �擪�P��̔����m�� pw1[w] �̐ݒ�
  sum=0;
  for(w=0 ; w<V ; w++){
    pw1[w] = drand48();
    sum += pw1[w];
  }
  for(w=0 ; w<V ; w++)
    pw1[w] /= sum;

  double p[V+1];
  double r;

  // ���i�P���j�Ƃ��̃g�s�b�N�̐���
  int words[600],z[600];
  int s;
  double Ps;
  for(m=0 ; m<M ; m++){
    words[0] = randomSelect(V,pw1);
    words[1] = randomSelect(V,pw1);
    words[2] = randomSelect(V,pw1);
    for(i=3 ; i<500 ; i++){
      words[i] = randomSelect(V+1,&(pw[words[i-1]][0]));
      if(words[i]==V) break;
    }
    if(i==500){
      fprintf(stderr,"making too long sentence!\n");
      exit(0);
    }

    int n=i;

    z[0] = randomSelect(T,&(phi[words[0]][0]));
    numW[words[0]]++;
    numWZ[words[0]][z[0]]++;

    for(i=1 ; i<n ; i++){
      s = randomSelect(2,&(theta[words[i-1]][words[i]][0]));
      if(s==0){
	z[i] = randomSelect(T,&(zeta[words[i-1]][words[i]][0]));
	numWW0[words[i-1]][words[i]]++;
	numWWZ[words[i-1]][words[i]][z[i]]++;
      }
      else{
	z[i] = randomSelect(T,&(phi[words[i]][0]));
	numWW1[words[i-1]][words[i]]++;
	numW[words[i]]++;
	numWZ[words[i]][z[i]]++;
      }
    }
	 
    
    t=0;
    for(i=0 ; i<n ; i++)
      t = (t | type[z[i]]);
    
    if(m<10)          printf("000000%d 8",m);
    else if(m<100)    printf("00000%d 8",m);
    else if(m<1000)   printf("0000%d 8",m);
    else if(m<10000)  printf("000%d 8",m);
    else if(m<100000) printf("00%d 8",m);
    
    
    for(i=1 ; i<=6 ; i++)
      printf(" %d",type2v[t][i]);
    
    for(i=0 ; i<n ; i++)
      printf(" w%d",words[i]);
    
    printf("\n");    
  }

  // �p�����^�̍Ŗސ���
  fp=fopen("MLmodel-theta","w");
  for(w1=0 ; w1<V ; w1++)
    for(w2=0 ; w2<V ; w2++){
      theta[w1][w2][0] = (double)numWW0[w1][w2]/(numWW0[w1][w2]+numWW1[w1][w2]);
      if(theta[w1][w2][0]>0.1)
	fprintf(fp,"w%d w%d 0 %f\n",w1,w2,theta[w1][w2][0]);
    }
  fclose(fp);

  fp=fopen("MLmodel-phi","w");
  for(w=0 ; w<V ; w++){
    for(i=1 ; i<T ; i++){
      phi[w][i]=(double)numWZ[w][i]/numW[w];
      if(phi[w][i]>0.1)
	fprintf(fp,"w%d %d %f\n",w,i,phi[w][i]);
    }
  }
  fclose(fp);

  fp=fopen("MLmodel-zeta","w");
  for(w1=0 ; w1<V ; w1++)
    for(w2=0 ; w2<V ; w2++){
      for(i=1 ; i<T ; i++){
	zeta[w1][w2][i]=(double)numWWZ[w1][w2][i]/numWW0[w1][w2];
	if(zeta[w1][w2][i]>0.1)
	  fprintf(fp,"w%d w%d %d %f\n",w1,w2,i,zeta[w1][w2][i]);
      }
    }
  fclose(fp);
}


