#include<stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <complex.h>

//サイズの指定
#define INIT 2560
//以下bmpライブラリ
#define PIXEL_NUM_X (INIT)/* 画像のXサイズ */
#define PIXEL_NUM_Y (INIT)/* 画像のYサイズ */
#define COLOR_BIT (24)/* 色ビット */
#define PIC_DATA_SIZE (PIXEL_NUM_X * 3 * PIXEL_NUM_Y)/* bitmapのサイズ */

struct point{
	long double cr;
	long double ci;
	long double nr;
	long double ni;
	long double z;
};

int count(struct point p);

/* 画像ライブラリ - プロトタイプ宣言 */
void createPic(unsigned char bitmap[INIT][INIT],unsigned char *b,int red,int green,int blue);
int putBmpHeader(FILE *s, int x, int y, int c);
int fputc2LowHigh(unsigned short d, FILE *s);
int fputc4LowHigh(unsigned long d, FILE *s);



int main(void){
	//URLの取得
	char *str;
	//開始座標
	float get_x=-1.600000,get_y=-1.200000;
	//拡大率
	unsigned long int get_per=100;
	//マンデルブロ集合の色
	int get_red=255,get_green=255,get_blue=255;
	if((str = getenv("QUERY_STRING"))!=NULL){
		//printf("%s\n",str);
		if(strcmp(str, "") != 0){
			sscanf(str,"x=%f&y=%f&per=%ld&red=%d&green=%d&blue=%d",&get_x,&get_y,&get_per,&get_red,&get_green,&get_blue);
		}
	}
	//strcpy(str,(const char *)getenv("QUERY_STRING"));
	unsigned char bitmap[INIT][INIT];
	long double split = (2.4/INIT)*((long double)100/(long double)get_per);
	for(int y=0;y<INIT;y++){
		for(int x=0;x<INIT;x++){
			long double point_x = get_x+split*(long double) x;//実数軸
			long double point_y = get_y+split*(long double) y;//虚数軸
			struct point p = {point_x,point_y,0,0,0};
			int c = count(p);//計算回数
			bitmap[y][x] = c;
		}
	}

	//画像用
	FILE *fd;
	unsigned char *b;
	//ここからライブラリ
	b = malloc(PIC_DATA_SIZE);
	createPic(bitmap,b,get_red,get_green,get_blue);
	fd = fopen("mandelbrot_h.bmp", "wb");
	putBmpHeader(fd, PIXEL_NUM_X, PIXEL_NUM_Y, COLOR_BIT);
	fwrite(b, sizeof(unsigned char), PIC_DATA_SIZE, fd);
	fclose(fd);

	//HTML表示用
		printf("Content-type: text/html; charset=UTF-8\n\n");
		printf("<html>\n");
		//リダイレクト
		printf("<meta http-equiv=\"refresh\" content=\"0;URL=/src/mandelbrot_h.bmp\">");
		printf("</html>\n");
	return 0;
}

int count(struct point p){
	int cnt = 0;
	while(p.z< 2  && cnt<255){
		double _Complex z = (p.nr*p.nr)-(p.ni*p.ni)+p.cr+(2*p.nr*p.ni+p.ci)*I;
		//実数部の取り出し
		double real = creal(z);
		//虚数部の取り出し
		double imaginary = cimag(z);
		p.z = real+imaginary;
		p.nr = real;
		p.ni = imaginary;
		cnt++;
	}
	return cnt;
}

/* 画像生成関数
 * http://c-language-program.cocolog-nifty.com/blog/2010/03/bmp-4b2c.html
 * */
void createPic(unsigned char bitmap[INIT][INIT],unsigned char *b,int get_red,int get_green,int get_blue)
{
	unsigned char red,green,blue,red_r,green_r,blue_r;
	// 乱数
	srand(time(NULL));

	red_r = rand() % 256;
	green_r = rand() % 256;
	blue_r = rand() % 256;

	for(int y=0;y<INIT;y++){
		for(int x=0;x<INIT;x++){

			unsigned char hassan = bitmap[y][x];
			//printf("%3d:",hassan);
			red = (unsigned char) red_r - hassan;
			green = (unsigned char) green_r - hassan;
			blue = (unsigned char) blue_r - hassan;
			//集合の色
			if(hassan >= 255){
				red = get_red;
				green = get_green;
				blue = get_blue;
			}
			*b = red;//red
			b++;
			*b = green;//green
			b++;
			*b = blue;//blue
			b++;
		}
		//printf("\n");
	}
}
/*
	putBmpHeader BMPヘッダ書出

	BMPファイルのヘッダを書き出す

	●戻り値
		int:0…失敗, 1…成功
	●引数
		FILE *s:[i] 出力ストリーム
		int x:[i] 画像Xサイズ(dot, 1〜)
		int y:[i] 画像Yサイズ(dot, 1〜)
		int c:[i] 色ビット数(bit/dot, 1 or 4 or 8 or 24)
*/
int putBmpHeader(FILE *s, int x, int y, int c)
{
	int i;
	int color; /* 色数 */
	unsigned long int bfOffBits; /* ヘッダサイズ(byte) */

	/* 画像サイズが異常の場合,エラーでリターン */
	if (x <= 0 || y <= 0) {
		return 0;
	}

	/* 出力ストリーム異常の場合,エラーでリターン */
	if (s == NULL || ferror(s)) {
		return 0;
	}

	/* 色数を計算 */
	if (c == 24) {
		color = 0;
	} else {
		color = 1;
		for (i=1;i<=c;i++) {
			color *= 2;
		}
	}

	/* ヘッダサイズ(byte)を計算 */
	/* ヘッダサイズはビットマップファイルヘッダ(14) + ビットマップ情報ヘッダ(40) + 色数 */
	bfOffBits = 14 + 40 + 4 * color;

	/* ビットマップファイルヘッダ(計14byte)を書出 */
	/* 識別文字列 */
	fputs("BM", s);

	/* bfSize ファイルサイズ(byte) */
	fputc4LowHigh(bfOffBits + (unsigned long)x * y, s);

	/* bfReserved1 予約領域1(byte) */
	fputc2LowHigh(0, s);

	/* bfReserved2 予約領域2(byte) */
	fputc2LowHigh(0, s);

	/* bfOffBits ヘッダサイズ(byte) */
	fputc4LowHigh(bfOffBits, s);

	/* ビットマップ情報ヘッダ(計40byte) */
	/* biSize 情報サイズ(byte) */
	fputc4LowHigh(40, s);

	/* biWidth 画像Xサイズ(dot) */
	fputc4LowHigh(x, s);

	/* biHeight 画像Yサイズ(dot) */
	fputc4LowHigh(y, s);

	/* biPlanes 面数 */
	fputc2LowHigh(1, s);

	/* biBitCount 色ビット数(bit/dot) */
	fputc2LowHigh(c, s);

	/* biCompression 圧縮方式 */
	fputc4LowHigh(0, s);

	/* biSizeImage 圧縮サイズ(byte) */
	fputc4LowHigh(0, s);

	/* biXPelsPerMeter 水平解像度(dot/m) */
	fputc4LowHigh(0, s);

	/* biYPelsPerMeter 垂直解像度(dot/m) */
	fputc4LowHigh(0, s);

	/* biClrUsed 色数 */
	fputc4LowHigh(0, s);

	/* biClrImportant 重要色数 */
	fputc4LowHigh(0, s);

	/* 書出失敗ならエラーでリターン */
	if (ferror(s)) {
		return 0;
	}

	/* 成功でリターン */
	return 1;
}

/*
	fputc2LowHigh 2バイトデータ書出(下位〜上位)

	2バイトのデータを下位〜上位の順で書き出す

	●戻り値
		int:EOF…失敗, EOF以外…成功
	●引数
		unsigned short d:[i] データ
		FILE *s:[i] 出力ストリーム
*/
int fputc2LowHigh(unsigned short d, FILE *s)
{
	putc(d & 0xFF, s);
	return putc(d >> CHAR_BIT, s);
}

/*
	fputc4LowHigh 4バイトデータ書出(下位〜上位)

	4バイトのデータを下位〜上位の順で書き出す

	●戻り値
		int:EOF…失敗, EOF以外…成功
	●引数
		unsigned long d:[i] データ
		FILE *s:[i] 出力ストリーム
*/
int fputc4LowHigh(unsigned long d, FILE *s)
{
	putc(d & 0xFF, s);
	putc((d >> CHAR_BIT) & 0xFF, s);
	putc((d >> CHAR_BIT * 2) & 0xFF, s);
	return putc((d >> CHAR_BIT * 3) & 0xFF, s);
}
