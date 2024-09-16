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
#define RECORDER_FILE "D:\\��������\\��������\\��߷�.txt"
#define PI 3.14159265359

//ͼƬ����͸����
void putimagePNG(int x, int y, IMAGE* picture) //xΪ����ͼƬ��X���꣬yΪY����
{
	// ������ʼ��
	DWORD* dst = GetImageBuffer();    // GetImageBuffer()���������ڻ�ȡ��ͼ�豸���Դ�ָ�룬EASYX�Դ�
	DWORD* draw = GetImageBuffer();
	DWORD* src = GetImageBuffer(picture); //��ȡpicture���Դ�ָ��
	int picture_width = picture->getwidth(); //��ȡpicture�Ŀ�ȣ�EASYX�Դ�
	int picture_height = picture->getheight(); //��ȡpicture�ĸ߶ȣ�EASYX�Դ�
	int graphWidth = getwidth();       //��ȡ��ͼ���Ŀ�ȣ�EASYX�Դ�
	int graphHeight = getheight();     //��ȡ��ͼ���ĸ߶ȣ�EASYX�Դ�
	int dstX = 0;    //���Դ������صĽǱ�

	// ʵ��͸����ͼ ��ʽ�� Cp=��p*FP+(1-��p)*BP �� ��Ҷ˹���������е���ɫ�ĸ��ʼ���
	for (int iy = 0; iy < picture_height; iy++)
	{
		for (int ix = 0; ix < picture_width; ix++)
		{
			int srcX = ix + iy * picture_width; //���Դ������صĽǱ�
			int sa = ((src[srcX] & 0xff000000) >> 24); //0xAArrggbb;AA��͸����
			int sr = ((src[srcX] & 0xff0000) >> 16); //��ȡRGB���R
			int sg = ((src[srcX] & 0xff00) >> 8);   //G
			int sb = src[srcX] & 0xff;              //B
			if (ix >= 0 && ix <= graphWidth && iy >= 0 && iy <= graphHeight && dstX <= graphWidth * graphHeight)
			{
				dstX = (ix + x) + (iy + y) * graphWidth; //���Դ������صĽǱ�
				int dr = ((dst[dstX] & 0xff0000) >> 16);
				int dg = ((dst[dstX] & 0xff00) >> 8);
				int db = dst[dstX] & 0xff;
				draw[dstX] = ((sr * sa / 255 + dr * (255 - sa) / 255) << 16)  //��ʽ�� Cp=��p*FP+(1-��p)*BP  �� ��p=sa/255 , FP=sr , BP=dr
					| ((sg * sa / 255 + dg * (255 - sa) / 255) << 8)         //��p=sa/255 , FP=sg , BP=dg
					| (sb * sa / 255 + db * (255 - sa) / 255);              //��p=sa/255 , FP=sb , BP=db
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
	void keyevent();//���ռ�������
	void Update();//����
	void drop();//���·���
	void clearline();//����
	void movingleftright(int offset);//�������ƶ�
	void movingupdown(int offset);//�������ƶ�
	void drawscore();//��ʾ����
	void savescore();//������߷֣������ļ�
	void displayover();//��������

	vector<vector<int>>map;
	int rows;
	int cols;
	int leftMargin;
	int topMargin;
	int blocksize;

	int score;//��ǰ����
	int highest;//��߷�

private:
	bool update;
	bool gameover;

};

class Block
{
public:
	Block();
	void movingleftright(int offset);//���Ʒ��������ƶ�
	void movingupdown(int offset);//���Ʒ��������ƶ�
	void draw(int leftMargin, int topMargin);//��С����
	static IMAGE** getImages();
	Block& operator=(const Block& other);
	bool blockInMap(const vector<vector<int>>&map);//�жϷ����Ƿ��������ڿհ׿���
	void solidify(vector<vector<int>>& map);//�̻�����
	bool checkIsOver(vector<vector<int>>& map);//�жϿհ������Ƿ񲻹���ǰ���飬��Ϸ�Ƿ����

private:
	int x;
	int y;
	int blocktype;
	Point smallblocks[4];
	IMAGE *img1;
	static IMAGE* imgs[12];
	static int size;

};

Block* curBlock;//��ǰ����
Block* nextBlock;//��һ����
Block bakBlock;//��ǰ���齵��ʱ�������ݺϷ�λ��

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


	DWORD dwError = mciSendString("open \"D:\\��������\\��������\\��Ч\\gamemusic.mp3\" type mpegvideo alias bkmusic",0,0,0);
	if (dwError) {
		char buffer[256];
		mciGetErrorString(dwError, buffer, sizeof(buffer));
		printf("Error: %s\n", buffer);
	}
	mciSendString("play bkmusic repeat", 0, 0, 0);

	//��ʼ����Ϸ������
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
	
	//��ʼ����߷�
	ifstream file(RECORDER_FILE);
	if (!file.is_open())
	{
		cout<< RECORDER_FILE <<"��ʧ��" << endl;
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
	init();//��ʼ��

	nextBlock = new Block;
	curBlock = nextBlock;
	nextBlock = new Block;

	Update();//����
	while (1)
	{
		keyevent();//���ܼ�������
		if (update)
		{
			update = false;
			Update();//����
			clearline();//����
		}

		if (curBlock->checkIsOver(map))//�ж��Ƿ�û�пհ�������Ϸ�Ƿ����
		{
			gameover = true;
		}

		if (gameover==true)
		{
			displayover();//����ҳ��
			savescore();//�������
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
		//����������ϼ����᷵��224 72
		//����������¼����᷵��224 80
		//���������������᷵��224 75
		//����������Ҽ����᷵��224 77
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
		//������»س����м����̻�����
		if (ch == 10 || ch == 13)
		{
			drop();
		}

		//��A��a������Ϸ
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
	loadimage(&imgBG, "D:\\��������\\��������\\ͼƬ\\����1.png", 960, 720);//���Ʊ���
	putimage(0,0, &imgBG);

	IMAGE** imgs = Block::getImages();
	for (int i = 0;i < rows;i++)
	{
		for (int j = 0;j < cols;j++)
		{
			if (map[i][j] == 0)continue;

			int x = j * blocksize + leftMargin;//��Ϸ����258-690
			int y = i * blocksize + topMargin;//��Ϸ������120-552
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
	
	//�ж���������
	for (int i = rows - 1;i >= 0;i--)
	{
		//����i���Ƿ�������
		int count = 0;
		for (int j = 0;j < cols;j++)
		{
			if (map[i][j])
			{
				count++;
			}
		}
		if (count < cols)//��������
		{
			continue;
		}
		else//����
		{
			/*
			for (int k = 0;k < cols;k++)
			{
				map[i][k] = { NULL };//����
			}
			*/
			lines1++;
		}
	}

	//�ж���������
	for (int i = cols - 1;i >= 0;i--)
	{
		//����i���Ƿ�������
		int count = 0;
		for (int j = 0;j < rows;j++)
		{
			if (map[j][i])
			{
				count++;
			}
		}
		if (count < rows)//��������
		{
			continue;
		}
		else//����
		{
			/*
			for (int k = 0;k < rows;k++)
			{
				map[k][i] = { NULL };//����
			}
			*/
			lines2++;
		}
	}
	
	//ʮ������
	if (lines1 > 0 && lines2 > 0)
	{
		for (int i = rows - 1;i >= 0;i--)
		{
			//����i���Ƿ�������
			int count = 0;
			for (int j = 0;j < cols;j++)
			{
				if (map[i][j])
				{
					count++;
				}
			}
			if (count < cols)//��������
			{
				continue;
			}
			else//����
			{
				for (int k = cols - 1;k >= 0;k--)
				{
					//����k���Ƿ�������
					int count = 0;
					for (int m = 0;m < rows;m++)
					{
						if (map[m][k])
						{
							count++;
						}
					}
					if (count < rows)//��������
					{
						continue;
					}
					else//���������У���i�е�k��
					{
						for (int a = 0;a < cols;a++)
						{
							map[i][a] = { NULL };//������
							//num[m] = a;

						}
						for (int b = 0;b < rows;b++)
						{
							map[b][k] = { NULL };//������
							//num1[n] = b;
						}
					}
				}
			}
		}
	}

	//������
	if (lines1 > 0 && lines2 == 0)
	{
		for (int i = rows - 1;i >= 0;i--)
		{
			//����i���Ƿ�������
			int count = 0;
			for (int j = 0;j < cols;j++)
			{
				if (map[i][j])
				{
					count++;
				}
			}
			if (count < cols)//��������
			{
				continue;
			}
			else//����
			{
				
				for (int k = 0;k < cols;k++)
				{
					map[i][k] = { NULL };//������
					//num[m] = k;
					//m++;
					//Sleep(200);
				}
			}
		}
	}
	
	//������
	if (lines2 > 0 && lines1 == 0)
	{
		for (int i = cols - 1;i >= 0;i--)
		{
			//����i���Ƿ�������
			int count = 0;
			for (int j = 0;j < rows;j++)
			{
				if (map[j][i])
				{
					count++;
				}
			}
			if (count < rows)//��������
			{
				continue;
			}
			else//����
			{
				
				for (int k = 0;k < rows;k++)
				{
					map[k][i] = { NULL };//������
					//num1[n] = k;
					//n++;
					//Sleep(200);
				}
			}
		}
	}


	IMAGE good, excellent;
	loadimage(&good, "D:\\��������\\��������\\ͼƬ\\good.png", 130, 130);
	loadimage(&excellent, "D:\\��������\\��������\\ͼƬ\\excellent.png", 130, 25);
	
	DWORD dwError = mciSendString("open \"D:\\��������\\��������\\��Ч\\bubble.mp3\" type mpegvideo alias bubble", 0, 0, 0);
	if (dwError) {
		char buffer[256];
		mciGetErrorString(dwError, buffer, sizeof(buffer));
		printf("Error: %s\n", buffer);
	}
	if (lines1 > 0||lines2>0)
	{
		//�ӷ�
		int addScore[4] = { 10,30,60,80 };
		score += addScore[lines1 + lines2-1];
		if (lines1 + lines2 - 1 == 0)
		{
			putimagePNG(430, 260, &good);//�÷ּ�10ʱ����ʾgood
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
			putimagePNG(430, 300, &excellent);//�÷ִ���10ʱ����ʾexcellent
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

	//���ʺ�
	loadimage(&rainbow, "D:\\��������\\��������\\ͼƬ\\rainbow2.png", 900, 110);
	putimagePNG(30, 50, &rainbow);
	
	//�����Ͻ���
	loadimage(&cloud, "D:\\��������\\��������\\ͼƬ\\cloud.png", 135, 100);
	putimagePNG(780, 0, &cloud);

	//��װ��pony
	loadimage(&pony1, "D:\\��������\\��������\\ͼƬ\\pony1.png");
	putimagePNG(780, 77, &pony1);

	loadimage(&pony2, "D:\\��������\\��������\\ͼƬ\\pony2.png",80,80);
	putimagePNG(715, 230, &pony2);


	char scoretext[32];
	//���Ƶ�ǰ����
	setbkmode(TRANSPARENT);
	settextcolor(WHITE);
	sprintf_s(scoretext, sizeof(scoretext), "CURRENT SCORE: %d", score);
	settextstyle(16, 0, "Fixedsys����");
	outtextxy(732, 152, scoretext);
	settextcolor(RGB(221, 160, 221));
	sprintf_s(scoretext, sizeof(scoretext), "CURRENT SCORE: %d", score);
	settextstyle(16, 0, "Fixedsys����");
	outtextxy(730, 150, scoretext);
	settextcolor(BLACK);
	sprintf_s(scoretext, sizeof(scoretext), "CURRENT SCORE: %d", score);
	settextstyle(16, 0, "Fixedsys����");
	outtextxy(728, 148, scoretext);


	//������߷�
	setbkmode(TRANSPARENT);
	settextcolor(WHITE);
	sprintf_s(scoretext, sizeof(scoretext), "HIGHEST SCORE: %d", highest);
	settextstyle(16, 0, "Fixedsys����");
	outtextxy(732, 302, scoretext);
	settextcolor(RGB(221,160,221));
	sprintf_s(scoretext, sizeof(scoretext), "HIGHEST SCORE: %d", highest);
	settextstyle(16, 0, "Fixedsys����");
	outtextxy(730, 300, scoretext);
	settextcolor(BLACK);
	sprintf_s(scoretext, sizeof(scoretext), "HIGHEST SCORE: %d", highest);
	settextstyle(16, 0, "Fixedsys����");
	outtextxy(728, 298, scoretext);

	//next block��ʾ
	setbkmode(TRANSPARENT);
	settextcolor(WHITE);
	settextstyle(16, 0, "Fixedsys����");
	outtextxy(142, 352, TEXT("NEXT BLOCK"));
	settextcolor(RGB(221, 160, 221));
	settextstyle(16, 0, "Fixedsys����");
	outtextxy(140, 350, TEXT("NEXT BLOCK"));
	settextcolor(BLACK);
	settextstyle(16, 0, "Fixedsys����");
	outtextxy(138, 348, TEXT("NEXT BLOCK"));


	//��ʾ��a������Ϸ
	setbkmode(TRANSPARENT);
	settextcolor(RGB(202,225,255));
	settextstyle(21, 0, "΢���ź�");
	outtextxy(800,36, TEXT("��A������Ϸ"));
	settextcolor(RGB(148, 0, 211));
	outtextxy(802,38, TEXT("��A������Ϸ"));
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
	loadimage(&finish, "D:\\��������\\��������\\ͼƬ\\���㱳��.png");

	Sleep(300);

	//��������
	mciSendString("close bkmusic", 0, 0, 0);
	DWORD dwError = mciSendString("open \"D:\\��������\\��������\\��Ч\\end.mp3\" type mpegvideo alias endmusic", 0, 0, 0);
	if (dwError) {
		char buffer[256];
		mciGetErrorString(dwError, buffer, sizeof(buffer));
		printf("Error: %s\n", buffer);
	}
	mciSendString("play endmusic", 0, 0, 0);
	
	//��������
	putimage(0, 0, &finish);

	setbkmode(TRANSPARENT);
	settextcolor(WHITE);
	settextstyle(40, 0, "Fixedsys����");
	outtextxy(352, 302, TEXT("YOUR SCORE"));
	settextcolor(RGB(221, 160, 221));
	settextstyle(40, 0, "Fixedsys����");
	outtextxy(350, 300, TEXT("YOUR SCORE"));
	settextcolor(BLACK);
	settextstyle(40, 0, "Fixedsys����");
	outtextxy(348, 298, TEXT("YOUR SCORE"));

	setbkmode(TRANSPARENT);//��ʾ���շ���
	settextcolor(WHITE);
	sprintf_s(scoretext, sizeof(scoretext), " %d", score);
	settextstyle(40, 0, "Fixedsys����");
	outtextxy(342, 382, scoretext);
	settextcolor(RGB(221, 160, 221));
	sprintf_s(scoretext, sizeof(scoretext), " %d", score);
	settextstyle(40, 0, "Fixedsys����");
	outtextxy(340, 380, scoretext);
	settextcolor(BLACK);
	sprintf_s(scoretext, sizeof(scoretext), " %d", score);
	settextstyle(40, 0, "Fixedsys����");
	outtextxy(338, 378, scoretext);

	if (score > highest)//������߷�new highest��ʾ
	{
		setbkmode(TRANSPARENT);
		settextcolor(WHITE);
		settextstyle(20, 0, "Fixedsys����");
		outtextxy(502, 392, TEXT("NEW HIGHEST"));
		settextcolor(RGB(221, 160, 221));
		settextstyle(20, 0, "Fixedsys����");
		outtextxy(500, 390, TEXT("NEW HIGHEST"));
		settextcolor(RGB(104,34,139));
		settextstyle(20, 0, "Fixedsys����");
		outtextxy(498, 388, TEXT("NEW HIGHEST"));
	}
}

IMAGE* Block::imgs[12] = { NULL, };
int Block::size = 54;

Block::Block()
{
	//��ʼ������
	if (imgs[0] == NULL)
	{
		IMAGE blockimg;
		loadimage(&blockimg, "D:\\��������\\��������\\ͼƬ\\block2.png",648,54);
		SetWorkingImage(&blockimg);
		for (int i = 0;i < 12;i++)
		{
			imgs[i] = new IMAGE;
			getimage(imgs[i], i * size, 0, size, size);
		}
		SetWorkingImage();//�ָ�������
		srand(time(NULL));
	}
	blocktype = 1 + rand() % 12;

	//���з�����״
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
	//ÿ�����鶼����һ����С������ɵ�
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
		//���ñ�ǣ����̻�����Ӧ��λ��
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

	//������ͼ���Աȵ�ǰ���飬�жϿհ������Ƿ��ܷ��µ�ǰ����
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
