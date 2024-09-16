#include<iostream>
#include<string>
#include<stdio.h>
#include<graphics.h>
#include <conio.h>
#include<time.h>
#include<stdlib.h>
#include<vector>
#include<windows.h>
#include <cstdarg>
#include<mmsystem.h>
#include<fstream>
#pragma comment(lib,"winmm.lib")

using namespace std;
#define RECORDER_FILE "D:\\消除方块\\消除方块\\最高分.txt"
#define PI 3.14159265359

//图片背景透明化
void putimagePNG(int x, int y, IMAGE* picture) //x为载入图片的X坐标，y为Y坐标
{
	// 变量初始化
	DWORD* dst = GetImageBuffer();    // GetImageBuffer()函数，用于获取绘图设备的显存指针，EASYX自带
	DWORD* draw = GetImageBuffer();
	DWORD* src = GetImageBuffer(picture); //获取picture的显存指针
	int picture_width = picture->getwidth(); //获取picture的宽度，EASYX自带
	int picture_height = picture->getheight(); //获取picture的高度，EASYX自带
	int graphWidth = getwidth();       //获取绘图区的宽度，EASYX自带
	int graphHeight = getheight();     //获取绘图区的高度，EASYX自带
	int dstX = 0;    //在显存里像素的角标

	// 实现透明贴图 公式： Cp=αp*FP+(1-αp)*BP ， 贝叶斯定理来进行点颜色的概率计算
	for (int iy = 0; iy < picture_height; iy++)
	{
		for (int ix = 0; ix < picture_width; ix++)
		{
			int srcX = ix + iy * picture_width; //在显存里像素的角标
			int sa = ((src[srcX] & 0xff000000) >> 24); //0xAArrggbb;AA是透明度
			int sr = ((src[srcX] & 0xff0000) >> 16); //获取RGB里的R
			int sg = ((src[srcX] & 0xff00) >> 8);   //G
			int sb = src[srcX] & 0xff;              //B
			if (ix >= 0 && ix <= graphWidth && iy >= 0 && iy <= graphHeight && dstX <= graphWidth * graphHeight)
			{
				dstX = (ix + x) + (iy + y) * graphWidth; //在显存里像素的角标
				int dr = ((dst[dstX] & 0xff0000) >> 16);
				int dg = ((dst[dstX] & 0xff00) >> 8);
				int db = dst[dstX] & 0xff;
				draw[dstX] = ((sr * sa / 255 + dr * (255 - sa) / 255) << 16)  //公式： Cp=αp*FP+(1-αp)*BP  ； αp=sa/255 , FP=sr , BP=dr
					| ((sg * sa / 255 + dg * (255 - sa) / 255) << 8)         //αp=sa/255 , FP=sg , BP=dg
					| (sb * sa / 255 + db * (255 - sa) / 255);              //αp=sa/255 , FP=sb , BP=db
			}
		}
	}
}


struct Point
{
	int row;
	int col;
};

class Gamecontrol
{
public:
	Gamecontrol(int row, int col, int left, int top, int blocksize);
	void init();
	void play();
	
private:
	void keyevent();//接收键盘输入
	void Update();//更新
	void drop();//放下方块
	void clearline();//消除
	void movingleftright(int offset);//向左右移动
	void movingupdown(int offset);//向上下移动
	void drawscore();//显示分数
	void savescore();//保存最高分，存入文件
	void displayover();//结束界面

	vector<vector<int>>map;
	int rows;
	int cols;
	int leftMargin;
	int topMargin;
	int blocksize;

	int score;//当前分数
	int highest;//最高分

private:
	bool update;
	bool gameover;

};

class Block
{
public:
	Block();
	void movingleftright(int offset);//控制方块左右移动
	void movingupdown(int offset);//控制方块上下移动
	void draw(int leftMargin, int topMargin);//画小方块
	static IMAGE** getImages();
	Block& operator=(const Block& other);
	bool blockInMap(const vector<vector<int>>&map);//判断方块是否在区域内空白块上
	void solidify(vector<vector<int>>& map);//固化方块
	bool checkIsOver(vector<vector<int>>& map);//判断空白区域是否不够当前方块，游戏是否结束

private:
	int x;
	int y;
	int blocktype;
	Point smallblocks[4];
	IMAGE *img1;
	static IMAGE* imgs[12];
	static int size;

};

Block* curBlock;//当前方块
Block* nextBlock;//下一方块
Block bakBlock;//当前方块降落时用来备份合法位置

Gamecontrol::Gamecontrol(int row, int col, int left, int top, int blocksize)
{
	this->rows = rows;
	this->cols = cols;
	this->leftMargin = left;
	this->topMargin = top;
	this->blocksize = blocksize;

	for (int i = 0;i < rows;i++)
	{
		vector<int>maprow;
		for (int j = 0;j < cols;j++)
		{
			maprow.push_back(0);
		}
		map.push_back(maprow);
	}
}

void Gamecontrol::init()
{
	srand(time(NULL));
	initgraph(960, 720);


	DWORD dwError = mciSendString("open \"D:\\消除方块\\消除方块\\音效\\gamemusic.mp3\" type mpegvideo alias bkmusic",0,0,0);
	if (dwError) {
		char buffer[256];
		mciGetErrorString(dwError, buffer, sizeof(buffer));
		printf("Error: %s\n", buffer);
	}
	mciSendString("play bkmusic repeat", 0, 0, 0);

	//初始化游戏区数据
	char data[8][8];
	rows = 8;
	cols = 8;
	
	for (int i = 0;i < rows;i++)
	{
		vector<int>tem;
		for (int j = 0;j < cols;j++)
		{
			tem.push_back(0);
		}
		map.push_back(tem);
	}

	score = 0;
	
	//初始化最高分
	ifstream file(RECORDER_FILE);
	if (!file.is_open())
	{
		cout<< RECORDER_FILE <<"打开失败" << endl;
		highest = 0;
	}
	else {
		file >> highest;
	}
	file.close();

	gameover = false;
}

void Gamecontrol::play()
{
	srand(time(NULL));
	init();//初始化

	nextBlock = new Block;
	curBlock = nextBlock;
	nextBlock = new Block;

	Update();//更新
	while (1)
	{
		keyevent();//接受键盘输入
		if (update)
		{
			update = false;
			Update();//更新
			clearline();//消除
		}

		if (curBlock->checkIsOver(map))//判断是否没有空白区域，游戏是否结束
		{
			gameover = true;
		}

		if (gameover==true)
		{
			displayover();//结束页面
			savescore();//保存分数
			system("pause");
		}
	}
}

void Gamecontrol::keyevent()
{
	unsigned char ch;//0-255
	int dx = 0;
	if (_kbhit())
	{
		ch = _getch();
		//如果按下向上键，会返回224 72
		//如果按下向下键，会返回224 80
		//如果按下向左键，会返回224 75
		//如果按下向右键，会返回224 77
		if (ch == 224)
		{
			ch = _getch();
			switch (ch) {
			case 72:
				dx = -1;
				break;
			case 80:
				dx = -2;
				break;
			case 75:
				dx = 1;
				break;
			case 77:
				dx = 2;
				break;
			default:
				break;
			}
		}
		//如果按下回车或换行键，固化方块
		if (ch == 10 || ch == 13)
		{
			drop();
		}

		//按A或a结束游戏
		if (ch == 65||ch==97)
		{
			gameover = true;
		}
	}
	if (dx != 0)
	{
		if (dx < 0)
		{
			if(dx == -1)
				movingupdown(-1);
			else
				movingupdown(1);
			update = true;

		}
		if (dx > 0)
		{
			if (dx == 1)
				movingleftright(-1);
			else
				movingleftright(1);
			update = true;

		}
	}
}

void Gamecontrol::Update()
{
	cleardevice();
	BeginBatchDraw();

	IMAGE imgBG;
	loadimage(&imgBG, "D:\\消除方块\\消除方块\\图片\\背景1.png", 960, 720);//绘制背景
	putimage(0,0, &imgBG);

	IMAGE** imgs = Block::getImages();
	for (int i = 0;i < rows;i++)
	{
		for (int j = 0;j < cols;j++)
		{
			if (map[i][j] == 0)continue;

			int x = j * blocksize + leftMargin;//游戏区宽：258-690
			int y = i * blocksize + topMargin;//游戏区长：120-552
			putimage(x, y, imgs[map[i][j] - 1]);
		}
	}
	curBlock->draw(leftMargin, topMargin);
	nextBlock->draw(90,390);

	drawscore();

	EndBatchDraw();
}

void Gamecontrol::drop()
{
	bakBlock = *curBlock;
	//char c = cin.get();
	//curBlock->drop();
	if ( curBlock->blockInMap(map))
	{
		bakBlock.solidify(map);
		delete curBlock;
		curBlock = nextBlock;
		nextBlock = new Block;

		//checkover();
	}
}

void Gamecontrol::clearline()
{
	int lines1 = 0;
	int lines2 = 0;
	
	//判断满行数量
	for (int i = rows - 1;i >= 0;i--)
	{
		//检查第i行是否是满行
		int count = 0;
		for (int j = 0;j < cols;j++)
		{
			if (map[i][j])
			{
				count++;
			}
		}
		if (count < cols)//不是满行
		{
			continue;
		}
		else//满行
		{
			/*
			for (int k = 0;k < cols;k++)
			{
				map[i][k] = { NULL };//消除
			}
			*/
			lines1++;
		}
	}

	//判断满列数量
	for (int i = cols - 1;i >= 0;i--)
	{
		//检查第i列是否是满列
		int count = 0;
		for (int j = 0;j < rows;j++)
		{
			if (map[j][i])
			{
				count++;
			}
		}
		if (count < rows)//不是满列
		{
			continue;
		}
		else//满列
		{
			/*
			for (int k = 0;k < rows;k++)
			{
				map[k][i] = { NULL };//消除
			}
			*/
			lines2++;
		}
	}
	
	//十字消除
	if (lines1 > 0 && lines2 > 0)
	{
		for (int i = rows - 1;i >= 0;i--)
		{
			//检查第i行是否是满行
			int count = 0;
			for (int j = 0;j < cols;j++)
			{
				if (map[i][j])
				{
					count++;
				}
			}
			if (count < cols)//不是满行
			{
				continue;
			}
			else//满行
			{
				for (int k = cols - 1;k >= 0;k--)
				{
					//检查第k列是否是满列
					int count = 0;
					for (int m = 0;m < rows;m++)
					{
						if (map[m][k])
						{
							count++;
						}
					}
					if (count < rows)//不是满列
					{
						continue;
					}
					else//满列又满行，第i行第k列
					{
						for (int a = 0;a < cols;a++)
						{
							map[i][a] = { NULL };//消除行
							//num[m] = a;

						}
						for (int b = 0;b < rows;b++)
						{
							map[b][k] = { NULL };//消除列
							//num1[n] = b;
						}
					}
				}
			}
		}
	}

	//消除行
	if (lines1 > 0 && lines2 == 0)
	{
		for (int i = rows - 1;i >= 0;i--)
		{
			//检查第i行是否是满行
			int count = 0;
			for (int j = 0;j < cols;j++)
			{
				if (map[i][j])
				{
					count++;
				}
			}
			if (count < cols)//不是满行
			{
				continue;
			}
			else//满行
			{
				
				for (int k = 0;k < cols;k++)
				{
					map[i][k] = { NULL };//消除行
					//num[m] = k;
					//m++;
					//Sleep(200);
				}
			}
		}
	}
	
	//消除列
	if (lines2 > 0 && lines1 == 0)
	{
		for (int i = cols - 1;i >= 0;i--)
		{
			//检查第i列是否是满列
			int count = 0;
			for (int j = 0;j < rows;j++)
			{
				if (map[j][i])
				{
					count++;
				}
			}
			if (count < rows)//不是满列
			{
				continue;
			}
			else//满列
			{
				
				for (int k = 0;k < rows;k++)
				{
					map[k][i] = { NULL };//消除列
					//num1[n] = k;
					//n++;
					//Sleep(200);
				}
			}
		}
	}


	IMAGE good, excellent;
	loadimage(&good, "D:\\消除方块\\消除方块\\图片\\good.png", 130, 130);
	loadimage(&excellent, "D:\\消除方块\\消除方块\\图片\\excellent.png", 130, 25);
	
	DWORD dwError = mciSendString("open \"D:\\消除方块\\消除方块\\音效\\bubble.mp3\" type mpegvideo alias bubble", 0, 0, 0);
	if (dwError) {
		char buffer[256];
		mciGetErrorString(dwError, buffer, sizeof(buffer));
		printf("Error: %s\n", buffer);
	}
	if (lines1 > 0||lines2>0)
	{
		//加分
		int addScore[4] = { 10,30,60,80 };
		score += addScore[lines1 + lines2-1];
		if (lines1 + lines2 - 1 == 0)
		{
			putimagePNG(430, 260, &good);//得分加10时，提示good
			DWORD dwError = mciSendString("play bubble wait", 0, 0, 0);
			if (dwError) {
				char buffer[256];
				mciGetErrorString(dwError, buffer, sizeof(buffer));
				printf("Error: %s\n", buffer);
			}
			Sleep(200);
		}
		if (lines1 + lines2 - 1 > 0)
		{
			putimagePNG(430, 300, &excellent);//得分大于10时，提示excellent
			mciSendString("play bubble wait", 0, 0, 0);
			Sleep(200);
		}
		lines1 = 0;
		lines2 = 0;
		update = true;
	}
	mciSendString("close bubble", 0, 0, 0);
}

void Gamecontrol::movingleftright(int offset)
{
	bakBlock = *curBlock;
	curBlock->movingleftright(offset);
	
}

void Gamecontrol::movingupdown(int offset)
{
	bakBlock = *curBlock;
	curBlock->movingupdown(offset);
	
}

void Gamecontrol::drawscore()
{
	IMAGE pony1, pony2;
	IMAGE rainbow;
	IMAGE cloud;

	//画彩虹
	loadimage(&rainbow, "D:\\消除方块\\消除方块\\图片\\rainbow2.png", 900, 110);
	putimagePNG(30, 50, &rainbow);
	
	//画左上角云
	loadimage(&cloud, "D:\\消除方块\\消除方块\\图片\\cloud.png", 135, 100);
	putimagePNG(780, 0, &cloud);

	//画装饰pony
	loadimage(&pony1, "D:\\消除方块\\消除方块\\图片\\pony1.png");
	putimagePNG(780, 77, &pony1);

	loadimage(&pony2, "D:\\消除方块\\消除方块\\图片\\pony2.png",80,80);
	putimagePNG(715, 230, &pony2);


	char scoretext[32];
	//绘制当前分数
	setbkmode(TRANSPARENT);
	settextcolor(WHITE);
	sprintf_s(scoretext, sizeof(scoretext), "CURRENT SCORE: %d", score);
	settextstyle(16, 0, "Fixedsys常规");
	outtextxy(732, 152, scoretext);
	settextcolor(RGB(221, 160, 221));
	sprintf_s(scoretext, sizeof(scoretext), "CURRENT SCORE: %d", score);
	settextstyle(16, 0, "Fixedsys常规");
	outtextxy(730, 150, scoretext);
	settextcolor(BLACK);
	sprintf_s(scoretext, sizeof(scoretext), "CURRENT SCORE: %d", score);
	settextstyle(16, 0, "Fixedsys常规");
	outtextxy(728, 148, scoretext);


	//绘制最高分
	setbkmode(TRANSPARENT);
	settextcolor(WHITE);
	sprintf_s(scoretext, sizeof(scoretext), "HIGHEST SCORE: %d", highest);
	settextstyle(16, 0, "Fixedsys常规");
	outtextxy(732, 302, scoretext);
	settextcolor(RGB(221,160,221));
	sprintf_s(scoretext, sizeof(scoretext), "HIGHEST SCORE: %d", highest);
	settextstyle(16, 0, "Fixedsys常规");
	outtextxy(730, 300, scoretext);
	settextcolor(BLACK);
	sprintf_s(scoretext, sizeof(scoretext), "HIGHEST SCORE: %d", highest);
	settextstyle(16, 0, "Fixedsys常规");
	outtextxy(728, 298, scoretext);

	//next block提示
	setbkmode(TRANSPARENT);
	settextcolor(WHITE);
	settextstyle(16, 0, "Fixedsys常规");
	outtextxy(142, 352, TEXT("NEXT BLOCK"));
	settextcolor(RGB(221, 160, 221));
	settextstyle(16, 0, "Fixedsys常规");
	outtextxy(140, 350, TEXT("NEXT BLOCK"));
	settextcolor(BLACK);
	settextstyle(16, 0, "Fixedsys常规");
	outtextxy(138, 348, TEXT("NEXT BLOCK"));


	//提示按a结束游戏
	setbkmode(TRANSPARENT);
	settextcolor(RGB(202,225,255));
	settextstyle(21, 0, "微软雅黑");
	outtextxy(800,36, TEXT("按A结束游戏"));
	settextcolor(RGB(148, 0, 211));
	outtextxy(802,38, TEXT("按A结束游戏"));
}

void Gamecontrol::savescore()
{
	if (score > highest)
	{
		highest = score;
		ofstream file(RECORDER_FILE);
		file << highest;
		file.close();
	}
}

void Gamecontrol::displayover()
{
	IMAGE finish;
	char scoretext[32];
	loadimage(&finish, "D:\\消除方块\\消除方块\\图片\\结算背景.png");

	Sleep(300);

	//结束音乐
	mciSendString("close bkmusic", 0, 0, 0);
	DWORD dwError = mciSendString("open \"D:\\消除方块\\消除方块\\音效\\end.mp3\" type mpegvideo alias endmusic", 0, 0, 0);
	if (dwError) {
		char buffer[256];
		mciGetErrorString(dwError, buffer, sizeof(buffer));
		printf("Error: %s\n", buffer);
	}
	mciSendString("play endmusic", 0, 0, 0);
	
	//结束界面
	putimage(0, 0, &finish);

	setbkmode(TRANSPARENT);
	settextcolor(WHITE);
	settextstyle(40, 0, "Fixedsys常规");
	outtextxy(352, 302, TEXT("YOUR SCORE"));
	settextcolor(RGB(221, 160, 221));
	settextstyle(40, 0, "Fixedsys常规");
	outtextxy(350, 300, TEXT("YOUR SCORE"));
	settextcolor(BLACK);
	settextstyle(40, 0, "Fixedsys常规");
	outtextxy(348, 298, TEXT("YOUR SCORE"));

	setbkmode(TRANSPARENT);//显示最终分数
	settextcolor(WHITE);
	sprintf_s(scoretext, sizeof(scoretext), " %d", score);
	settextstyle(40, 0, "Fixedsys常规");
	outtextxy(342, 382, scoretext);
	settextcolor(RGB(221, 160, 221));
	sprintf_s(scoretext, sizeof(scoretext), " %d", score);
	settextstyle(40, 0, "Fixedsys常规");
	outtextxy(340, 380, scoretext);
	settextcolor(BLACK);
	sprintf_s(scoretext, sizeof(scoretext), " %d", score);
	settextstyle(40, 0, "Fixedsys常规");
	outtextxy(338, 378, scoretext);

	if (score > highest)//超过最高分new highest提示
	{
		setbkmode(TRANSPARENT);
		settextcolor(WHITE);
		settextstyle(20, 0, "Fixedsys常规");
		outtextxy(502, 392, TEXT("NEW HIGHEST"));
		settextcolor(RGB(221, 160, 221));
		settextstyle(20, 0, "Fixedsys常规");
		outtextxy(500, 390, TEXT("NEW HIGHEST"));
		settextcolor(RGB(104,34,139));
		settextstyle(20, 0, "Fixedsys常规");
		outtextxy(498, 388, TEXT("NEW HIGHEST"));
	}
}

IMAGE* Block::imgs[12] = { NULL, };
int Block::size = 54;

Block::Block()
{
	//初始化方块
	if (imgs[0] == NULL)
	{
		IMAGE blockimg;
		loadimage(&blockimg, "D:\\消除方块\\消除方块\\图片\\block2.png",648,54);
		SetWorkingImage(&blockimg);
		for (int i = 0;i < 12;i++)
		{
			imgs[i] = new IMAGE;
			getimage(imgs[i], i * size, 0, size, size);
		}
		SetWorkingImage();//恢复工作区
		srand(time(NULL));
	}
	blocktype = 1 + rand() % 12;

	//所有方块形状
	int blocks[12][4] = {
		1,3,5,7,
		2,4,5,7,
		3,5,4,6,
		3,5,4,7,
		2,4,6,7,
		3,5,7,6,
		2,3,4,5,
		1,1,1,1,
		1,1,3,3,
		1,1,3,2,
		2,2,4,5,
		2,3,2,3,
	};
	for (int i = 0;i < 4;i++)
	{
		smallblocks[i].row = blocks[blocktype - 1][i] / 2;
		smallblocks[i].col = blocks[blocktype - 1][i] % 2;
	}
	img1 = imgs[blocktype - 1];

	
}

void Block::draw(int leftMargin,int topMargin)
{
	//每个方块都是由一个个小方块组成的
	for (int i = 0;i < 4;i++)
	{
		int x = leftMargin + smallblocks[i].col * size;
		int y = topMargin + smallblocks[i].row * size;
		putimage(x, y, img1);
	}
}

IMAGE**Block:: getImages()
{
	return imgs;
}

Block& Block::operator=(const Block& other)
 {
	 if (this == &other) return *this;

	 this->blocktype = other.blocktype;
	 for (int i = 0;i < 4;i++)
	 {
		 this->smallblocks[i] = other.smallblocks[i];
	 }
	 return *this;

 }

bool Block::blockInMap(const vector<vector<int>>& map)
{
	if (map.empty())
		return false;
	int rows = map.size();
	int cols = map[0].size();
	for (int i = 0;i < 4;i++)
	{
		if (smallblocks[i].col < 0 || smallblocks->col >= cols ||
			smallblocks[i].row < 0 || smallblocks[i].row >= rows ||
			(smallblocks[i].row >= 0 && smallblocks[i].row < rows && smallblocks[i].col>=0 && smallblocks[i].col<cols && map[smallblocks[i].row][smallblocks[i].col]))
		{
			return false;
		}
	}
	return true;
}

void Block::solidify(vector<vector<int>>& map)
{
	for (int i = 0;i < 4;i++)
	{
		//设置标记，“固化”对应的位置
		map[smallblocks[i].row][smallblocks[i].col] = blocktype;
	}
}

void Block::movingleftright(int offset)
{
	for (int i = 0;i < 4;i++)
	{
		smallblocks[i].col += offset;
	}
}

void Block::movingupdown(int offset)
{
	for (int i = 0;i < 4;i++)
	{
		smallblocks[i].row += offset;
	}
}

bool Block::checkIsOver(vector<vector<int>>& map)
{
	int rowmin = curBlock->smallblocks[0].row < curBlock->smallblocks[1].row ? curBlock->smallblocks[0].row : curBlock->smallblocks[1].row;
	rowmin = rowmin < curBlock->smallblocks[2].row ? rowmin : curBlock->smallblocks[2].row;
	rowmin = rowmin < curBlock->smallblocks[3].row ? rowmin : curBlock->smallblocks[3].row;

	int colmin = curBlock->smallblocks[0].col > curBlock->smallblocks[1].col ? curBlock->smallblocks[0].col : curBlock->smallblocks[1].col;
	colmin = colmin < curBlock->smallblocks[2].col ? colmin : curBlock->smallblocks[2].col;
	colmin = colmin < curBlock->smallblocks[3].col ? colmin : curBlock->smallblocks[3].col;

	Block tem = {};

	//遍历地图，对比当前方块，判断空白区域是否能放下当前方块
	for (int i = 0; i < (int)map.size()-1; i++)
	{
		for (int j = 0;j < (int)map[0].size()-1;j++)
		{
			for (int k = 0; k < 4; k++)
			{
				tem.smallblocks[k].row = curBlock->smallblocks[k].row + i - rowmin;
				tem.smallblocks[k].col = curBlock->smallblocks[k].col + j - colmin;
			}			
			if (tem.blockInMap(map))
			{
				return false;
			}
		}
	}
	return true;
}

int main()
{
	
	Gamecontrol game(8, 8, 258, 120, 54);
	//game.init();
	game.play();
	return 0;
}
